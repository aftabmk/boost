#pragma once

#include <iostream>
#include <memory_resource>
#include <vector>

#include <cache_engine/cache_engine.h>

#include "benchmark_config.h"
#include "benchmark_stats.h"
#include "benchmark_single_thread.h"
#include "benchmark_multi_thread.h"
#include "benchmark_utils.h"

namespace bench {

// ============================================================================
//  Benchmark Runner
// ============================================================================

template <size_t Stages = Config::kStages, 
          size_t SlotsPerStage = Config::kSlotsPerStage>
class BenchmarkRunner {
public:
    using CacheType = cache_engine::Cache<Stages, SlotsPerStage>;
    using Arena = std::vector<std::byte>;

    explicit BenchmarkRunner(const Config& config = Config{}) 
        : config_(config) {}

    void run() {
        print_header();

        for (size_t payload_size : config_.payload_sizes) {
            run_payload_size(payload_size);
        }

        print_footer();
    }

private:
    void print_header() {
        IosFlagGuard guard(std::cout);
        std::cout << "=== Cache Engine Benchmark ===\n"
                  << "ops:         " << std::dec << config_.num_ops << "\n"
                  << "threads:     " << std::dec << config_.get_num_threads() << "\n"
                  << "read ratio:  " << std::dec << config_.read_ratio_pct << "%\n"
                  << "stages:      " << std::dec << Stages << "\n"
                  << "slots/stage: " << std::dec << SlotsPerStage << "\n"
                  << "memory:      " << std::dec << config_.memory_mb << " MB\n\n";
    }

    void print_footer() {
        std::cout << "=== Benchmark complete ===\n";
    }

    void run_payload_size(size_t payload_size) {
        std::cout << std::dec << "--- Payload size: " << payload_size << " bytes ---\n";

        // Fresh arena per payload size
        Arena arena(config_.memory_mb * 1024 * 1024);
        std::pmr::monotonic_buffer_resource resource(arena.data(), arena.size());
        CacheType cache(resource);

        std::string value = make_value(payload_size);

        // Cap SET ops to pool size
        size_t set_ops = std::min<size_t>(config_.num_ops, Config::kTotalSlots);

        // 1. SET benchmark
        run_set_benchmark(cache, set_ops, value);

        // 2. GET benchmark
        run_get_benchmark(cache, set_ops);

        // 3. Mixed workload
        run_mixed_benchmark(value);

        std::cout << "\n";
    }

    void run_set_benchmark(CacheType& cache, size_t num_ops, const std::string& value) {
        std::cout << "[1] SET benchmark\n";
        auto t0 = clock_type::now();
        auto stats = bench_set(cache, num_ops, value);
        auto t1 = clock_type::now();
        print_stats("set()", stats, 
                    std::chrono::duration<double>(t1 - t0).count());
    }

    void run_get_benchmark(CacheType& cache, size_t num_ops) {
        std::cout << "[2] GET benchmark (existing keys)\n";
        size_t hits = 0, misses = 0;
        auto t0 = clock_type::now();
        auto stats = bench_get(cache, num_ops, hits, misses);
        auto t1 = clock_type::now();
        print_stats("get()", stats,
                    std::chrono::duration<double>(t1 - t0).count());
        {
            IosFlagGuard guard(std::cout);
            std::cout << "    hits: " << std::dec << hits
                      << "  misses: " << std::dec << misses << "\n";
        }
    }

    void run_mixed_benchmark(const std::string& value) {
        std::cout << "[3] Mixed workload (" << config_.get_num_threads() 
                  << " threads, " << config_.read_ratio_pct << "% reads)\n";

        // Fresh arena for mixed workload
        Arena mixed_arena(config_.memory_mb * 1024 * 1024);
        std::pmr::monotonic_buffer_resource mixed_res(mixed_arena.data(),
                                                       mixed_arena.size());
        CacheType mixed_cache(mixed_res);

        size_t num_keys = std::min<size_t>(4096, Config::kTotalSlots);
        size_t ops_per_thread = config_.num_ops / config_.get_num_threads();

        auto mixed = bench_mixed(mixed_cache, num_keys, ops_per_thread,
                                  config_.get_num_threads(), 
                                  config_.read_ratio_pct, value);

        double ops_per_sec = mixed.elapsed_sec > 0
                              ? static_cast<double>(mixed.total_ops) / mixed.elapsed_sec
                              : 0;
        {
            IosFlagGuard guard(std::cout);
            std::cout << "    total ops:   " << std::dec << mixed.total_ops << "\n"
                      << "    reads:       " << std::dec << mixed.reads
                      << " (hits=" << std::dec << mixed.hits
                      << ", misses=" << std::dec << mixed.misses << ")\n"
                      << "    writes:      " << std::dec << mixed.writes << "\n"
                      << std::fixed << std::setprecision(3)
                      << "    elapsed:     " << mixed.elapsed_sec << " s\n"
                      << std::setprecision(0)
                      << "    throughput:  " << ops_per_sec << " ops/sec\n";
        }
    }

    Config config_;
};

} // namespace bench