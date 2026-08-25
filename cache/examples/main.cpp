#include <cache_engine/cache_engine.h>
#include <memory_resource>
#include <iostream>
#include <vector>
#include <exception>

int main() {
    try {
        std::cout << "=== Cache Engine Example ===" << std::endl;
        std::cout << "Step 1: Allocating 64MB memory..." << std::endl;
        
        std::vector<std::byte> memory(1024 * 1024 * 64);
        std::pmr::monotonic_buffer_resource resource(memory.data(), memory.size());
        
        std::cout << "Step 2: Creating cache (4 stages, 8 slots per stage)..." << std::endl;
        // Use smaller cache: 4 stages, 8 slots per stage = 32 slots total
        cache_engine::Cache<4, 8> cache(resource);
        std::cout << "  Cache created successfully" << std::endl;
        
        std::cout << "Step 3: Writing data..." << std::endl;
        
        bool success = cache.set("username", "john_doe");
        std::cout << "  set username: " << (success ? "OK" : "FAIL") << std::endl;
        
        success = cache.set("email", "john@example.com");
        std::cout << "  set email: " << (success ? "OK" : "FAIL") << std::endl;
        
        success = cache.set("age", "30");
        std::cout << "  set age: " << (success ? "OK" : "FAIL") << std::endl;
        
        success = cache.set("score", "1000");
        std::cout << "  set score: " << (success ? "OK" : "FAIL") << std::endl;
        
        std::cout << "Step 4: Reading data..." << std::endl;
        
        auto username = cache.get("username");
        if (username) {
            std::cout << "  username: " << *username << std::endl;
        } else {
            std::cout << "  username: NOT FOUND" << std::endl;
        }
        
        auto email = cache.get("email");
        if (email) {
            std::cout << "  email: " << *email << std::endl;
        } else {
            std::cout << "  email: NOT FOUND" << std::endl;
        }
        
        auto age = cache.get("age");
        if (age) {
            std::cout << "  age: " << *age << std::endl;
        } else {
            std::cout << "  age: NOT FOUND" << std::endl;
        }
        
        auto score = cache.get("score");
        if (score) {
            std::cout << "  score: " << *score << std::endl;
        } else {
            std::cout << "  score: NOT FOUND" << std::endl;
        }
        
        std::cout << "=== DONE ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "UNKNOWN EXCEPTION" << std::endl;
        return 1;
    }
    
    return 0;
}