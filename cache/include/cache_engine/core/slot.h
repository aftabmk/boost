#pragma once

#include <atomic>
#include <cstdint>
#include <cstddef>

namespace cache_engine {

/**
 * 64-byte aligned slot.
 * 
 * On Windows/MSVC, std::atomic<uint32_t> is 8 bytes aligned.
 * Total size may be 64 or 128 depending on platform.
 * We only enforce alignment, not exact size.
 */
struct alignas(64) Slot {
    // Seqlock: even = stable, odd = write in progress
    std::atomic<uint32_t> seq{0};
    
    // Bumped on every write/reuse
    uint32_t generation{0};
    
    // Key identity (64-bit hash)
    uint64_t key_hash{0};
    
    // Ring buffer pointer (payload storage)
    void* ring_buffer{nullptr};
    
    // Payload metadata
    uint32_t payload_size{0};
    uint8_t head{0};        // Ring index
    uint32_t version{0};    // MVCC version
};

// Only enforce alignment, not exact size
static_assert(alignof(Slot) == 64, "Slot must be 64-byte aligned");

// Check size is reasonable (64-128 bytes)
static_assert(sizeof(Slot) >= 64 && sizeof(Slot) <= 128, 
              "Slot must be between 64 and 128 bytes");

} // namespace cache_engine