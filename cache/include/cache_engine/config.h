#pragma once

#include <cstddef>
#include <cstdint>

namespace cache_engine {

// Cache topology
static constexpr size_t kStages = 16;
static constexpr size_t kSlotsPerStage = 1024;
static constexpr size_t kTotalSlots = kStages * kSlotsPerStage;

// Ring configuration
static constexpr size_t kMinChunkSize = 64;
static constexpr size_t kMaxChunkSize = 1024;  // 1MB
static constexpr uint8_t kDefaultDepth = 4;

// Performance
static constexpr int kMaxRetries = 16;

// Free list
static constexpr size_t kLocalCacheSize = 16;

} // namespace cache_engine