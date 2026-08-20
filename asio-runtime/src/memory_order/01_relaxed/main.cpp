#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> counter{0};

void worker() {
    for (int i = 0; i < 100'000; ++i) {
        // Atomic increment; no synchronization with other data.
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    // Both threads contributed 100,000 increments.
    std::cout << "counter = "<< counter.load(std::memory_order_relaxed)<< '\n';
}