#include <boost/lockfree/spsc_queue.hpp>

#include <iostream>
#include <thread>

using Queue = boost::lockfree::spsc_queue<int,boost::lockfree::capacity<8>>;

Queue queue;

void producer() {
    for (int i = 0; i < 100; ++i) {
        // Keep trying until the consumer frees a slot.
        while (!queue.push(i)) {
            std::this_thread::yield();
        }
    }
}

void consumer() {
    int value;

    for (int count = 0; count < 100; ) {
        if (queue.pop(value)) {
            std::cout << "received: "<< value << '\n';

            ++count;
        } 
		else {
            // Give the producer a chance to run.
            std::this_thread::yield();
        }
    }
}

int main() {
    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    consumer_thread.join();
}