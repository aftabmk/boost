#pragma once

#include <cstddef>
#include <vector>
#include <thread>

namespace bench {

// Default configuration
struct Config {
    size_t num_ops = 20;
    unsigned num_threads = 0;  // 0 = auto-detect
    int read_ratio_pct = 95;
    
    // Payload sizes to test
    std::vector<size_t> payload_sizes = {8, 64, 512, 4096};
    
    // Memory pool size in MB
    size_t memory_mb = 64;
    
    // Cache configuration
    static constexpr size_t kStages = 16;
    static constexpr size_t kSlotsPerStage = 1024;
    static constexpr size_t kTotalSlots = kStages * kSlotsPerStage;
    
    // Auto-detect threads
    unsigned get_num_threads() const {
        if (num_threads == 0) {
            return std::max(1u, std::thread::hardware_concurrency());
        }
        return num_threads;
    }
};

} // namespace bench