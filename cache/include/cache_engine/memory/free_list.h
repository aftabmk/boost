#pragma once

#include <atomic>
#include <cstdint>
#include <boost/lockfree/queue.hpp>

namespace cache_engine {

/**
 * Lock-free slot pool using Boost.Lockfree queue.
 * 
 * Cross-platform (Linux, Windows, macOS).
 * No blocking, no mutexes.
 */
class FreeList {
public:
    explicit FreeList(size_t total_slots) 
        : queue_(total_slots) {
        // Initialize with all slot indices
        for (uint32_t i = 0; i < total_slots; ++i) {
            queue_.push(i);
        }
        available_.store(total_slots);
    }
    
    /**
     * Acquire a slot.
     * Returns 0xFFFFFFFF if none available.
     */
    uint32_t acquire() {
        uint32_t slot;
        if (queue_.pop(slot)) {
            available_.fetch_sub(1, std::memory_order_relaxed);
            return slot;
        }
        return 0xFFFFFFFF;
    }
    
    /**
     * Release a slot back to the pool.
     */
    void release(uint32_t slot_idx) {
        queue_.push(slot_idx);
        available_.fetch_add(1, std::memory_order_relaxed);
    }
    
    /**
     * Get number of available slots.
     */
    size_t available() const {
        return available_.load(std::memory_order_relaxed);
    }
    
private:
    boost::lockfree::queue<uint32_t> queue_;
    std::atomic<size_t> available_{0};
};

} // namespace cache_engine