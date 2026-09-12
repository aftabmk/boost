#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <memory_resource>

#include "seqlock.h"
#include "../config.h"

namespace cache_engine {

template <uint8_t Depth = kDefaultDepth>
class PayloadRing {
public:
    static constexpr uint8_t kDepth = Depth;
    static constexpr size_t kMinChunkSize = 64;
    static constexpr int kMaxRetries = 16;
    static constexpr size_t kMaxChunkSize = 1024 * 1024;  // 1MB limit
    
    struct Metadata {
        size_t chunk_size;
        std::atomic<uint32_t> write_head{0};
        std::atomic<uint32_t> version_counter{0};
    };
    
    struct Chunk {
        std::atomic<uint32_t> seq{0};
        uint32_t length{0};
        uint32_t version{0};
        uint8_t data[];
    };
    
    static void* allocate(std::pmr::memory_resource& resource, size_t payload_size) {
        size_t chunk_size = round_up_power_of_two(payload_size);
        if (chunk_size < kMinChunkSize) chunk_size = kMinChunkSize;
        if (chunk_size > kMaxChunkSize) chunk_size = kMaxChunkSize;
        
        size_t total_size = sizeof(Metadata) + (chunk_size * Depth);
        void* mem = resource.allocate(total_size, 64);
        
        auto* meta = static_cast<Metadata*>(mem);
        new (meta) Metadata{chunk_size, 0, 0};
        
        uint8_t* chunks = static_cast<uint8_t*>(mem) + sizeof(Metadata);
        std::memset(chunks, 0, chunk_size * Depth);
        
        return mem;
    }
    
    static uint8_t write(void* ring_ptr, std::string_view payload, uint32_t version) {
        auto* meta = static_cast<Metadata*>(ring_ptr);
        uint8_t* chunks = static_cast<uint8_t*>(ring_ptr) + sizeof(Metadata);
        
        uint32_t old_head = meta->write_head.load(std::memory_order_acquire);
        uint8_t new_head = static_cast<uint8_t>((old_head + 1) % Depth);
        
        Chunk* chunk = reinterpret_cast<Chunk*>(chunks + (new_head * meta->chunk_size));
        
        Seqlock lock(chunk->seq);
        uint32_t pre = lock.begin_write();
        
        std::memcpy(chunk->data, payload.data(), payload.size());
        chunk->length = static_cast<uint32_t>(payload.size());
        chunk->version = version;
        
        lock.end_write(pre);
        
        meta->write_head.store(new_head, std::memory_order_release);
        meta->version_counter.fetch_add(1, std::memory_order_release);
        
        return new_head;
    }
    
    /**
     * ReadResult uses PMR string - allocated from the same pool.
     * This prevents stack overflow and keeps memory in the PMR pool.
     */
    struct ReadResult {
        bool ok{false};
        uint32_t version{0};
        uint32_t length{0};
        std::pmr::string data;  // Uses PMR allocator
        
        // Constructor takes the PMR resource
        explicit ReadResult(std::pmr::memory_resource* resource = nullptr) 
            : data(resource ? resource : std::pmr::get_default_resource()) {}
    };
    
    /**
     * Read with PMR memory resource.
     */
    static ReadResult read(void* ring_ptr, uint8_t head, uint32_t expected_version,
                           std::pmr::memory_resource* resource) {
        auto* meta = static_cast<Metadata*>(ring_ptr);
        uint8_t* chunks = static_cast<uint8_t*>(ring_ptr) + sizeof(Metadata);
        
        if (head >= Depth) return ReadResult(resource);
        
        Chunk* chunk = reinterpret_cast<Chunk*>(chunks + (head * meta->chunk_size));
        Seqlock lock(chunk->seq);
        
        for (int attempt = 0; attempt < kMaxRetries; ++attempt) {
            auto seq_start = lock.try_begin_read();
            if (!seq_start) continue;
            
            ReadResult result(resource);
            result.version = chunk->version;
            result.length = chunk->length;
            
            // Allocate from PMR pool
            if (result.length > 0) {
                result.data.resize(result.length);
                std::memcpy(result.data.data(), chunk->data, result.length);
            }
            
            if (!lock.validate_read(*seq_start)) continue;
            
            if (result.version != expected_version) {
                return ReadResult(resource);
            }
            
            result.ok = true;
            return result;
        }
        return ReadResult(resource);
    }
    
private:
    static size_t round_up_power_of_two(size_t v) {
        if (v <= 64) return 64;
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;
        return v;
    }
};

} // namespace cache_engine