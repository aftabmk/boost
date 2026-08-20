#include <boost/lockfree/queue.hpp>

#include <atomic>
#include <iostream>
#include <thread>

using Queue = boost::lockfree::queue<int,boost::lockfree::capacity<64>>;

Queue queue;

std::atomic<int> produced{0};
std::atomic<int> consumed{0};

constexpr int producer_count = 3;
constexpr int items_per_producer = 100;
constexpr int total_items = producer_count * items_per_producer;

void producer(int id) {
    for (int i = 0; i < items_per_producer; ++i) {
        int value = id * 1000 + i;

        // Retry until this item is successfully inserted.
        while (!queue.push(value)) {
            std::this_thread::yield();
        }

        // This counter only tracks statistics.
        produced.fetch_add(1,std::memory_order_relaxed);
    }
}

void consumer(int id) {
    int value;

    while (consumed.load(std::memory_order_relaxed) < total_items) {
        if (queue.pop(value)) {
            int previous = consumed.fetch_add(1,std::memory_order_relaxed);

            if (previous < total_items) {
                std::cout << "worker " << id<< " processed "<< value << '\n';
            }
        } 
		else {
            // Queue may temporarily be empty while producers work.
            std::this_thread::yield();
        }
    }
}

int main() {
    std::thread producer1(producer, 1);
    std::thread producer2(producer, 2);
    std::thread producer3(producer, 3);

    std::thread consumer1(consumer, 1);
    std::thread consumer2(consumer, 2);
    std::thread consumer3(consumer, 3);

    producer1.join();
    producer2.join();
    producer3.join();

    consumer1.join();
    consumer2.join();
    consumer3.join();

    std::cout << "\nProduced: "<< produced.load()<< '\n';

    std::cout << "Consumed: "<< consumed.load()<< '\n';
}