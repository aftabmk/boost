#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <random>
#include <chrono>

#include <cache_engine/cache_engine.h>

#include "benchmark_utils.h"

namespace bench {

using clock_type = std::chrono::steady_clock;

// ============================================================================
//  Multi-Threaded Mixed Workload
// ============================================================================

template <typename CacheT>
struct MixedResult {
    size_t total_ops = 0;
    size_t reads = 0;
    size_t writes = 0;
    size_t hits = 0;
    size_t misses = 0;
    double elapsed_sec = 0;
};

template <typename CacheT>
MixedResult<CacheT> bench_mixed(CacheT& cache, size_t num_keys, 
                                  size_t ops_per_thread,
                                  unsigned num_threads, int read_ratio_pct,
                                  const std::string& value) {
    // Pre-populate keys
    for (size_t i = 0; i < num_keys; ++i) {
        cache.set(make_key(i), value);
    }

    std::atomic<size_t> total_reads{0};
    std::atomic<size_t> total_writes{0};
    std::atomic<size_t> total_hits{0};
    std::atomic<size_t> total_misses{0};

    auto worker = [&](unsigned thread_id) {
        std::mt19937 rng(thread_id * 7919u + 1);
        std::uniform_int_distribution<int> op_dist(1, 100);
        std::uniform_int_distribution<size_t> key_dist(0, num_keys - 1);

        size_t local_reads = 0, local_writes = 0, local_hits = 0, local_misses = 0;

        for (size_t i = 0; i < ops_per_thread; ++i) {
            size_t idx = key_dist(rng);
            std::string key = make_key(idx);

            if (op_dist(rng) <= read_ratio_pct) {
                auto result = cache.get(key);
                if (result.has_value()) ++local_hits;
                else ++local_misses;
                ++local_reads;
            } else {
                cache.set(key, value);
                ++local_writes;
            }
        }

        total_reads += local_reads;
        total_writes += local_writes;
        total_hits += local_hits;
        total_misses += local_misses;
    };

    auto t0 = clock_type::now();
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (unsigned t = 0; t < num_threads; ++t) {
        threads.emplace_back(worker, t);
    }
    for (auto& t : threads) t.join();
    auto t1 = clock_type::now();

    MixedResult<CacheT> res;
    res.reads = total_reads;
    res.writes = total_writes;
    res.hits = total_hits;
    res.misses = total_misses;
    res.total_ops = res.reads + res.writes;
    res.elapsed_sec = std::chrono::duration<double>(t1 - t0).count();
    return res;
}

} // namespace bench