#include <boost/lockfree/spsc_queue.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using Queue = boost::lockfree::spsc_queue<int,boost::lockfree::capacity<4096>>;

constexpr int item_count = 1'000'000;

Queue queue;

void producer() {
    for (int i = 0; i < item_count; ++i) {
        // Retry until the item enters the bounded queue.
        while (!queue.push(i)) {
            std::this_thread::yield();
        }
    }
}

void consumer() {
    int value;
    int count = 0;

    while (count < item_count) {
        if (queue.pop(value)) {
            // Successfully consumed one item.
            ++count;
        } 
        else {
            std::this_thread::yield();
        }
    }
}

int main() {
    auto start = std::chrono::steady_clock::now();

    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    consumer_thread.join();

    auto end = std::chrono::steady_clock::now();

    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "SPSC\n";
    std::cout << "items: " << item_count << '\n';
    std::cout << "time: "<< elapsed.count() << " ms\n";
}

// SPSC
// items: 1000000
// time: 5 ms