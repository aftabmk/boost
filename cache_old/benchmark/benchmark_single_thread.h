#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <iostream>

#include <cache_engine/cache_engine.h>

#include "benchmark_stats.h"
#include "benchmark_utils.h"

namespace bench {

using clock_type = std::chrono::steady_clock;

// ============================================================================
//  Single-Threaded Benchmarks
// ============================================================================

template <typename CacheT>
LatencyStats bench_set(CacheT& cache, size_t num_ops, const std::string& value) {
    std::vector<double> samples;
    samples.reserve(num_ops);

    for (size_t i = 0; i < num_ops; ++i) {
        std::string key = make_key(i);
        auto t0 = clock_type::now();
        bool ok = cache.set(key, value);
        auto t1 = clock_type::now();
        if (!ok) {
            std::cerr << "  WARNING: set() failed at i=" << i
                      << " (pool likely exhausted)\n";
            break;
        }
        samples.push_back(
            std::chrono::duration<double, std::nano>(t1 - t0).count());
    }
    return compute_stats(samples);
}

template <typename CacheT>
LatencyStats bench_get(CacheT& cache, size_t num_ops, 
                       size_t& hits, size_t& misses) {
    std::vector<double> samples;
    samples.reserve(num_ops);
    hits = 0;
    misses = 0;

    for (size_t i = 0; i < num_ops; ++i) {
        std::string key = make_key(i);
        auto t0 = clock_type::now();
        auto result = cache.get(key);
        auto t1 = clock_type::now();
        if (result.has_value()) ++hits;
        else ++misses;
        samples.push_back(
            std::chrono::duration<double, std::nano>(t1 - t0).count());
    }
    return compute_stats(samples);
}

} // namespace bench