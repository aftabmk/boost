#pragma once

#include <memory_resource>
#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>

namespace cache_engine {

/**
 * Power-of-2 slab allocator.
 * 
 * Fixed sizes: 64, 128, 256, 512, 1024, 2048, 4096, 8192,
 * 16384, 32768, 65536, 131072, 262144, 524288, 1048576
 * 
 * O(1) allocation and deallocation.
 * Batches allocations to reduce fragmentation.
 */
class SlabAllocator {
public:
    explicit SlabAllocator(std::pmr::memory_resource& resource) 
        : resource_(&resource) {
        for (size_t i = 0; i < kSlabCount; ++i) {
            slabs_[i].chunk_size = kSlabSizes[i];
            slabs_[i].free_list = std::pmr::vector<void*>(&resource);
            slabs_[i].free_list.reserve(256);
            slabs_[i].allocations = 0;
        }
    }
    
    /**
     * Allocate memory of at least 'size' bytes.
     * Rounds up to nearest slab size.
     */
    void* allocate(size_t size) {
        size_t slab_idx = get_slab_index(size);
        if (slab_idx == SIZE_MAX) {
            return resource_->allocate(size, 64);
        }
        
        auto& slab = slabs_[slab_idx];
        
        // Fast path: reuse from free list
        if (!slab.free_list.empty()) {
            void* ptr = slab.free_list.back();
            slab.free_list.pop_back();
            slab.allocations++;
            return ptr;
        }
        
        // Slow path: allocate batch of chunks
        size_t batch_size = 16;
        size_t block_size = slab.chunk_size * batch_size;
        void* ptr = resource_->allocate(block_size, 64);
        
        // Split into chunks
        char* start = static_cast<char*>(ptr);
        for (size_t i = 0; i < batch_size; ++i) {
            char* chunk = start + (i * slab.chunk_size);
            slab.free_list.push_back(chunk);
        }
        
        // Allocate one
        void* result = slab.free_list.back();
        slab.free_list.pop_back();
        slab.allocations++;
        return result;
    }
    
    /**
     * Deallocate memory.
     * Returns to free list if capacity allows.
     */
    void deallocate(void* ptr, size_t size) {
        size_t slab_idx = get_slab_index(size);
        if (slab_idx == SIZE_MAX) {
            resource_->deallocate(ptr, size);
            return;
        }
        
        auto& slab = slabs_[slab_idx];
        
        // Keep free list bounded
        if (slab.free_list.size() < 256) {
            slab.free_list.push_back(ptr);
            slab.allocations--;
        } else {
            resource_->deallocate(ptr, size);
        }
    }
    
private:
    // Slab sizes: power-of-2 from 64 to 1MB
    static constexpr size_t kSlabSizes[] = {
        64, 128, 256, 512, 1024, 2048, 4096, 8192,
        16384, 32768, 65536, 131072, 262144, 524288, 1048576
    };
    static constexpr size_t kSlabCount = sizeof(kSlabSizes) / sizeof(size_t);
    
    struct Slab {
        size_t chunk_size;
        std::pmr::vector<void*> free_list;
        size_t allocations;
    };
    
    std::array<Slab, kSlabCount> slabs_;
    std::pmr::memory_resource* resource_;
    
    /**
     * Find slab index for given size.
     * Returns SIZE_MAX if size too large.
     */
    size_t get_slab_index(size_t size) const {
        for (size_t i = 0; i < kSlabCount; ++i) {
            if (size <= kSlabSizes[i]) {
                return i;
            }
        }
        return SIZE_MAX;
    }
};

} // namespace cache_engine