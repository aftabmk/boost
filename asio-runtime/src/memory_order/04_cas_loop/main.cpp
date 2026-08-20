#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> counter{0};

void increment() {
    for (int i = 0; i < 100'000; ++i) {
        int expected = counter.load(std::memory_order_relaxed);

        while (!counter.compare_exchange_weak(
            expected,
            expected + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed
        )) {
            // expected is updated when CAS fails; retry with new value.
        }
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    std::thread t3(increment);
    std::thread t4(increment);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    // Four threads × 100,000 successful increments.
    std::cout << "counter = "<< counter.load(std::memory_order_relaxed)<< '\n';
}