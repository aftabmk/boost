// benchmark.cpp
//
// Benchmark harness for cache_engine::Cache.
//
// Measures:
//   1. Single-threaded SET throughput + latency percentiles
//   2. Single-threaded GET throughput + latency percentiles
//   3. Multi-threaded mixed read/write throughput (configurable ratio)
//   4. Sweep across payload sizes (small keys/values -> larger blobs)
//
// Usage:
//   benchmark [num_ops] [num_threads] [read_ratio_percent]
//
//   benchmark                 -> defaults: 200k ops, auto threads, 90% reads
//   benchmark 500000 8 95     -> 500k ops, 8 threads, 95% reads

#include <iostream>
#include <cstdlib>

#include "benchmark_config.h"
#include "benchmark_runner.h"

int main(int argc, char** argv) {
    // Parse command line arguments
    bench::Config config;
    
    if (argc > 1) {
        config.num_ops = static_cast<size_t>(std::atoll(argv[1]));
    }
    if (argc > 2) {
        config.num_threads = static_cast<unsigned>(std::atoi(argv[2]));
    }
    if (argc > 3) {
        config.read_ratio_pct = std::atoi(argv[3]);
    }

    // Run benchmark
    bench::BenchmarkRunner<16, 1024> runner(config);
    runner.run();

    return 0;
}