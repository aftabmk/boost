#include <boost/lockfree/queue.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

using Queue = boost::lockfree::queue<int,boost::lockfree::capacity<4096>>;

constexpr int item_count = 1'000'000;
constexpr int producer_count = 2;
constexpr int consumer_count = 2;

Queue queue;

std::atomic<int> produced{0};
std::atomic<int> consumed{0};

void producer() {
    while (true) {
        // Atomically reserve the next item to produce.
        int index = produced.fetch_add(1,std::memory_order_relaxed);

        if (index >= item_count) {
            break;
        }

        // Keep trying while the bounded queue is full.
        while (!queue.push(index)) {
            std::this_thread::yield();
        }
    }
}

void consumer() {
    int value;

    while (true) {
        int current = consumed.load(
            std::memory_order_relaxed
        );

        if (current >= item_count) {
            break;
        }

        if (queue.pop(value)) {
            // Claim this consumed item.
            int previous = consumed.fetch_add(1,std::memory_order_relaxed);

            if (previous >= item_count) {
                break;
            }
        } 
        else {
            std::this_thread::yield();
        }
    }
}

int main() {
    auto start = std::chrono::steady_clock::now();

    std::thread producers[producer_count];
    std::thread consumers[consumer_count];

    for (int i = 0; i < producer_count; ++i) {
        producers[i] = std::thread(producer);
    }

    for (int i = 0; i < consumer_count; ++i) {
        consumers[i] = std::thread(consumer);
    }

    for (auto& thread : producers) {
        thread.join();
    }

    for (auto& thread : consumers) {
        thread.join();
    }

    auto end = std::chrono::steady_clock::now();

    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "MPMC\n";
    std::cout << "items: " << item_count << '\n';
    std::cout << "time: "<< elapsed.count()<< " ms\n";
}

// MPMC
// items: 1000000
// time: 136 ms