#include <atomic>
#include <iostream>
#include <thread>

int data = 0;

std::atomic<bool> ready{false};

void producer() {
    // Normal write happens before the release operation.
    data = 42;

    // Publish the write to another thread.
    ready.store(true, std::memory_order_release);
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {
        // Wait until producer publishes the data.
    }

    // Acquire observes the producer's release.
    std::cout << "data = " << data << '\n';
}

int main() {
    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    consumer_thread.join();
}