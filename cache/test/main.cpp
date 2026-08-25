#include <cache_engine/cache_engine.h>
#include <memory_resource>
#include <iostream>
#include <cassert>
#include <vector>
#include <exception>

using namespace cache_engine;

// Use smaller cache for tests
using TestCache = Cache<4, 8>;

void test_set_get() {
    std::cout << "Testing set/get... " << std::flush;
    
    try {
        std::vector<std::byte> memory(1024 * 1024 * 64);
        std::pmr::monotonic_buffer_resource resource(memory.data(), memory.size());
        TestCache cache(resource);
        
        cache.set("key", "value");
        auto result = cache.get("key");
        
        if (!result.has_value()) {
            std::cout << "FAILED: result is null" << std::endl;
            return;
        }
        if (*result != "value") {
            std::cout << "FAILED: expected 'value', got '" << *result << "'" << std::endl;
            return;
        }
        std::cout << "PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

// ... rest of tests similar ...

int main() {
    std::cout << "=== Running Cache Engine Tests ===" << std::endl;
    std::cout << "==================================" << std::endl << std::endl;
    std::cout.flush();
    
    test_set_get();
    // ... call other tests ...
    
    std::cout << std::endl << "=== All tests complete ===" << std::endl;
    return 0;
}