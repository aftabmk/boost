#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <memory_resource>

#include "../core/slot.h"
#include "../core/seqlock.h"
#include "../core/ring.h"
#include "../config.h"

namespace cache_engine {

class CacheReader {
public:
    explicit CacheReader(const Slot* slots, std::pmr::memory_resource* resource = nullptr)
        : slots_(slots)
        , resource_(resource ? resource : std::pmr::get_default_resource()) {}

    struct ReadResult {
        bool ok{false};
        uint64_t key_hash{0};
        void* ring_buffer{nullptr};
        uint32_t payload_size{0};
        uint8_t head{0};
        uint32_t version{0};
    };

    ReadResult read(uint32_t slot_idx) const {
        const Slot& s = slots_[slot_idx];
        Seqlock lock(const_cast<std::atomic<uint32_t>&>(s.seq));

        for (int attempt = 0; attempt < kMaxRetries; ++attempt) {
            auto seq_start = lock.try_begin_read();
            if (!seq_start) continue;

            ReadResult result;
            result.key_hash = s.key_hash;
            result.ring_buffer = s.ring_buffer;
            result.payload_size = s.payload_size;
            result.head = s.head;
            result.version = s.version;

            if (!lock.validate_read(*seq_start)) continue;

            result.ok = true;
            return result;
        }
        return ReadResult{};
    }

    /**
     * Read string using PMR allocation.
     * Returns PMR string that uses the same memory pool.
     */
    std::pmr::string read_string_pmr(uint32_t slot_idx) const {
        ReadResult desc = read(slot_idx);
        if (!desc.ok || desc.ring_buffer == nullptr) return std::pmr::string(resource_);

        auto result = PayloadRing<>::read(desc.ring_buffer, desc.head, desc.version, resource_);
        if (!result.ok) return std::pmr::string(resource_);

        return std::move(result.data);  // Move PMR string
    }

    /**
     * Read string returning std::string (compatibility).
     * Converts from PMR string to std::string (copies).
     */
    struct StringReadResult {
        bool ok{false};
        std::string value;
    };

    StringReadResult read_string(uint32_t slot_idx) const {
        ReadResult desc = read(slot_idx);
        if (!desc.ok || desc.ring_buffer == nullptr) return {};

        auto result = PayloadRing<>::read(desc.ring_buffer, desc.head, desc.version, resource_);
        if (!result.ok) return {};

        // Convert PMR string to std::string
        return StringReadResult{
            true,
            std::string(result.data.data(), result.data.size())
        };
    }

private:
    const Slot* slots_;
    std::pmr::memory_resource* resource_;
};

} // namespace cache_engine