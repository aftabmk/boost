#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> counter{0};

void worker() {
    for (int i = 0; i < 100'000; ++i) {
        // Read-modify-write with both acquire and release semantics.
        counter.fetch_add(1,std::memory_order_acq_rel);
    }
}

int main() {
    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    std::cout << "counter = "<< counter.load(std::memory_order_acquire)<< '\n';
}

// this is default