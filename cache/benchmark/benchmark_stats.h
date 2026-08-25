#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

#include "benchmark_utils.h"

namespace bench {

// ============================================================================
//  Latency Statistics
// ============================================================================

struct LatencyStats {
    size_t count = 0;
    double min_ns = 0;
    double p50_ns = 0;
    double p95_ns = 0;
    double p99_ns = 0;
    double p999_ns = 0;
    double max_ns = 0;
    double mean_ns = 0;
};

// Compute statistics from latency samples
inline LatencyStats compute_stats(std::vector<double>& samples_ns) {
    LatencyStats s{};
    if (samples_ns.empty()) return s;

    std::sort(samples_ns.begin(), samples_ns.end());
    s.count = samples_ns.size();
    s.min_ns = samples_ns.front();
    s.max_ns = samples_ns.back();
    s.mean_ns = std::accumulate(samples_ns.begin(), samples_ns.end(), 0.0) / s.count;

    auto pct = [&](double p) -> double {
        size_t idx = static_cast<size_t>(p * (s.count - 1));
        return samples_ns[idx];
    };
    s.p50_ns = pct(0.50);
    s.p95_ns = pct(0.95);
    s.p99_ns = pct(0.99);
    s.p999_ns = pct(0.999);
    return s;
}

// Print statistics
inline void print_stats(const std::string& label, const LatencyStats& s, 
                        double elapsed_sec) {
    IosFlagGuard guard(std::cout);
    
    double ops_per_sec = elapsed_sec > 0 ? static_cast<double>(s.count) / elapsed_sec : 0;

    std::cout << "  " << label << "\n"
              << "    ops:        " << std::dec << s.count << "\n"
              << "    throughput: " << std::fixed << std::setprecision(0) 
              << ops_per_sec << " ops/sec\n"
              << std::defaultfloat
              << "    min:        " << fmt_ns(s.min_ns) << "\n"
              << "    mean:       " << fmt_ns(s.mean_ns) << "\n"
              << "    p50:        " << fmt_ns(s.p50_ns) << "\n"
              << "    p95:        " << fmt_ns(s.p95_ns) << "\n"
              << "    p99:        " << fmt_ns(s.p99_ns) << "\n"
              << "    p99.9:      " << fmt_ns(s.p999_ns) << "\n"
              << "    max:        " << fmt_ns(s.max_ns) << "\n";
}

} // namespace bench