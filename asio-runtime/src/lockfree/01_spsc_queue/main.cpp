#include <boost/lockfree/spsc_queue.hpp>

#include <iostream>
#include <thread>

using Queue = boost::lockfree::spsc_queue<int,boost::lockfree::capacity<1024>>;

Queue queue;

void producer() {
    for (int i = 0; i < 100; ++i) {
        while (!queue.push(i)) {
            // Retry while the bounded queue is full.
        }
    }
}

void consumer() {
    int value;

    for (int i = 0; i < 100; ) {
        if (queue.pop(value)) {
            // pop() succeeds only when an item is available.
            std::cout << "received: " << value << '\n';
            ++i;
        }
    }
}

int main() {
    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    consumer_thread.join();
}