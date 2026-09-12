#pragma once

/**
 * Cache Engine - High-performance lock-free cache.
 * 
 * Features:
 * - Lock-free seqlock protocol
 * - MVCC (Multi-Version Concurrency Control)
 * - Power-of-2 dynamic ring buffers
 * - Slab allocator for O(1) allocations
 * - Lock-free slot pool
 * - Header-only, cross-platform
 * 
 * Usage:
 *   alignas(64) static std::byte buffer[1024 * 1024 * 1024];
 *   std::pmr::monotonic_buffer_resource resource(buffer, sizeof(buffer));
 *   cache_engine::Cache<16, 1024> cache(resource);
 *   
 *   cache.set("key", "value");
 *   auto val = cache.get("key");
 */

#include <cstddef>
#include <memory_resource>
#include <string>
#include <optional>
#include <unordered_map>
#include <iostream>

#include "core/slot.h"
#include "cache/writer.h"
#include "cache/reader.h"
#include "hash/key_hash.h"
#include "config.h"

namespace cache_engine {

/**
 * Low-level cache engine.
 * Manages slot matrix, writer, reader.
 */
template <size_t Stages, size_t SlotsPerStage>
class CacheEngine {
public:
    static constexpr size_t kTotalSlots = Stages * SlotsPerStage;

    explicit CacheEngine(std::pmr::memory_resource& upstream)
        : slots_(static_cast<Slot*>(
              upstream.allocate(sizeof(Slot) * kTotalSlots, alignof(Slot))))
        , writer_(slots_, static_cast<uint32_t>(kTotalSlots), upstream)
        , reader_(slots_) {
        
        // Placement construct slots
        for (size_t i = 0; i < kTotalSlots; ++i) {
            new (&slots_[i]) Slot();
        }
    }

    CacheWriter& writer() { return writer_; }
    const CacheReader& reader() const { return reader_; }

private:
    Slot* slots_;
    CacheWriter writer_;
    CacheReader reader_;
};

/**
 * High-level cache API.
 * Provides simple set/get/remove operations.
 */
template <size_t Stages = kStages, size_t SlotsPerStage = kSlotsPerStage>
class Cache {
public:
    explicit Cache(std::pmr::memory_resource& resource)
        : engine_(resource) {}

    /**
     * Set key-value pair.
     * Returns true if successful.
     */
    bool set(std::string_view key, std::string_view value) {
        auto slot = slot_for(key);
        if (!slot) return false;
        engine_.writer().write_string(*slot, KeyHash::compute(key), value);
        return true;
    }

    /**
     * Get value for key.
     * Returns nullopt if key not found.
     */
    std::optional<std::string> get(std::string_view key) {
        auto it = slots_.find(std::string(key));
        if (it == slots_.end()) {
            std::cout << "get: key not found" << std::endl;
            return std::nullopt;
        }
        
        auto result = engine_.reader().read_string(it->second);
        if (!result.ok) {
            std::cout << "get: read_string failed" << std::endl;
            return std::nullopt;
        }

        return result.value;
    }

    /**
     * Remove key-value pair.
     * Returns true if key was found and removed.
     */
    bool remove(std::string_view key) {
        auto it = slots_.find(std::string(key));
        if (it == slots_.end()) return false;
        engine_.writer().write_tombstone(it->second);
        engine_.writer().release_slot(it->second);
        slots_.erase(it);
        return true;
    }

private:
    /**
     * Get or create slot for key.
     * Allocates new slot if key doesn't exist.
     */
    std::optional<uint32_t> slot_for(std::string_view key) {
        std::string k(key);
        if (auto it = slots_.find(k); it != slots_.end()) return it->second;
        auto slot = engine_.writer().acquire_slot();
        if (!slot) return std::nullopt;
        slots_.emplace(std::move(k), *slot);
        return slot;
    }

    CacheEngine<Stages, SlotsPerStage> engine_;
    std::unordered_map<std::string, uint32_t> slots_;
};

} // namespace cache_engine