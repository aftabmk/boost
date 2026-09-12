#pragma once

#include <cstdint>
#include <optional>
#include <memory_resource>
#include <string_view>
#include <unordered_map>
#include <iostream>  // Add for debugging

#include "../core/slot.h"
#include "../core/seqlock.h"
#include "../core/ring.h"
#include "../memory/slab_allocator.h"
#include "../memory/free_list.h"
#include "../config.h"

namespace cache_engine {

class CacheWriter {
public:
    CacheWriter(Slot* slots, uint32_t total_slots, 
                std::pmr::memory_resource& upstream)
        : slots_(slots)
        , resource_(&upstream)
        , slab_allocator_(upstream)
        , free_list_(total_slots) {}

    std::optional<uint32_t> acquire_slot() {
        uint32_t idx = free_list_.acquire();
        if (idx == 0xFFFFFFFF) return std::nullopt;
        return idx;
    }

    void release_slot(uint32_t idx) {
        free_list_.release(idx);
    }

    void write_string(uint32_t slot_idx, uint64_t key_hash, std::string_view payload) {
        Slot& s = slots_[slot_idx];
        
        void* ring = get_or_create_ring(key_hash, payload.size());
        
        uint32_t version = get_next_version(key_hash);
        uint8_t new_head = PayloadRing<>::write(ring, payload, version);
        
        Seqlock lock(s.seq);
        uint32_t pre = lock.begin_write();
        
        s.key_hash = key_hash;
        s.generation++;
        s.ring_buffer = ring;
        s.payload_size = static_cast<uint32_t>(payload.size());
        s.head = new_head;
        s.version = version;
        
        lock.end_write(pre);
    }

    void write_tombstone(uint32_t slot_idx) {
        Slot& s = slots_[slot_idx];
        
        Seqlock lock(s.seq);
        uint32_t pre = lock.begin_write();
        
        s.key_hash = 0;
        s.generation++;
        s.ring_buffer = nullptr;
        s.payload_size = 0;
        s.head = 0;
        s.version = 0;
        
        lock.end_write(pre);
    }

private:
    void* get_or_create_ring(uint64_t key_hash, size_t payload_size) {
        auto it = key_rings_.find(key_hash);
        if (it != key_rings_.end()) {
            return it->second;
        }
        
        void* ring = PayloadRing<>::allocate(*resource_, payload_size);
        key_rings_[key_hash] = ring;
        return ring;
    }

    uint32_t get_next_version(uint64_t key_hash) {
        return ++versions_[key_hash];
    }

    Slot* slots_;
    std::pmr::memory_resource* resource_;
    SlabAllocator slab_allocator_;
    FreeList free_list_;
    std::unordered_map<uint64_t, void*> key_rings_;
    std::unordered_map<uint64_t, uint32_t> versions_;
};

} // namespace cache_engine