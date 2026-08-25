#pragma once

#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace bench {

// ============================================================================
//  Random String Generator
// ============================================================================

inline std::string random_string(size_t length) {
    static const char charset[] = 
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);
    
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(rng)];
    }
    return result;
}

// ============================================================================
//  Key/Value Generators
// ============================================================================

inline std::string make_key(size_t i) {
    return "bench_key_" + std::to_string(i);
}

inline std::string make_value(size_t size_bytes) {
    return random_string(size_bytes);
}

// ============================================================================
//  Formatting
// ============================================================================

inline std::string fmt_ns(double ns) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    if (ns < 1000.0) {
        oss << ns << " ns";
    } else if (ns < 1'000'000.0) {
        oss << (ns / 1000.0) << " us";
    } else {
        oss << (ns / 1'000'000.0) << " ms";
    }
    return oss.str();
}

// ============================================================================
//  I/O Stream Guard
// ============================================================================

class IosFlagGuard {
public:
    explicit IosFlagGuard(std::ostream& os)
        : os_(os), flags_(os.flags()), precision_(os.precision()) {
        os_ << std::dec << std::noshowbase;
    }
    ~IosFlagGuard() {
        os_.flags(flags_);
        os_.precision(precision_);
    }
    IosFlagGuard(const IosFlagGuard&) = delete;
    IosFlagGuard& operator=(const IosFlagGuard&) = delete;

private:
    std::ostream& os_;
    std::ios_base::fmtflags flags_;
    std::streamsize precision_;
};

} // namespace bench