#pragma once

#include <cstdint>
#include <string_view>
#include <xxhash.h>

namespace cache_engine {

/**
 * XXHash wrapper for key hashing.
 * 
 * Uses XXH3_64bits for fast, high-quality hashing.
 * 64-bit hash with negligible collision probability.
 */
class KeyHash {
public:
    /**
     * Compute 64-bit hash of key.
     * Optional seed for hash table partitioning.
     */
    static uint64_t compute(std::string_view key, uint64_t seed = 0) {
        return XXH3_64bits_withSeed(key.data(), key.size(), seed);
    }
};

} // namespace cache_engine