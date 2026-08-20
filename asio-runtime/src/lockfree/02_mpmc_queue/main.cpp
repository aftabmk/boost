#include <boost/lockfree/queue.hpp>

#include <iostream>
#include <thread>

using Queue = boost::lockfree::queue<int,boost::lockfree::capacity<1024>>;

Queue queue;

void producer(int id) {
    for (int i = 0; i < 10; ++i) {
        int value = id * 100 + i;

        while (!queue.push(value)) {
            // Retry while the bounded queue is full.
        }
    }
}

void consumer(int id) {
    int value;

    for (int i = 0; i < 10; ) {
        if (queue.pop(value)) {
            std::cout << "consumer " << id << " received: " << value << '\n';

            ++i;
        }
    }
}

int main() {
    std::thread producer1(producer, 1);
    std::thread producer2(producer, 2);

    std::thread consumer1(consumer, 1);
    std::thread consumer2(consumer, 2);

    producer1.join();
    producer2.join();

    consumer1.join();
    consumer2.join();
}