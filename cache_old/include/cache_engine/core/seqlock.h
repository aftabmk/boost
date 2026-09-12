#pragma once

#include <atomic>
#include <cstdint>
#include <optional>

namespace cache_engine {

/**
 * Lock-free seqlock protocol.
 * 
 * Writers:
 *   1. Increment seq to odd (write in progress)
 *   2. Write data
 *   3. Increment seq to even (stable)
 * 
 * Readers:
 *   1. Read seq (must be even)
 *   2. Copy data
 *   3. Read seq again
 *   4. If same, data is consistent
 */
class Seqlock {
public:
    explicit Seqlock(std::atomic<uint32_t>& seq) : seq_(seq) {}
    
    /**
     * Begin write - returns pre-write sequence value.
     * Caller must pass this to end_write().
     */
    uint32_t begin_write() {
        uint32_t seq = seq_.load(std::memory_order_relaxed);
        seq_.store(seq + 1, std::memory_order_release);
        return seq;
    }
    
    /**
     * End write - pass pre-write sequence value.
     */
    void end_write(uint32_t pre_write_seq) {
        seq_.store(pre_write_seq + 2, std::memory_order_release);
    }
    
    /**
     * Begin read - returns sequence if not in progress.
     * Returns nullopt if write in progress (seq odd).
     */
    std::optional<uint32_t> try_begin_read() const {
        uint32_t s = seq_.load(std::memory_order_acquire);
        if (s & 1u) return std::nullopt;  // Odd = write in progress
        return s;
    }
    
    /**
     * Validate read - returns true if data is consistent.
     * Pass the sequence value from try_begin_read().
     */
    bool validate_read(uint32_t seq_at_start) const {
        uint32_t seq_now = seq_.load(std::memory_order_acquire);
        return seq_now == seq_at_start;
    }
    
private:
    std::atomic<uint32_t>& seq_;
};

} // namespace cache_engine