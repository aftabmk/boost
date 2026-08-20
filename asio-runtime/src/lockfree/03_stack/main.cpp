#include <boost/lockfree/stack.hpp>

#include <atomic>
#include <iostream>
#include <thread>

// Fixed-capacity lock-free stack.
// Multiple threads can push/pop concurrently.
using Stack = boost::lockfree::stack<int,boost::lockfree::capacity<1024>>;

Stack stack;

// Counts how many values have been successfully popped.
// relaxed is enough because this is only a counter.
std::atomic<int> consumed{0};

void producer(int id) {
    for (int i = 0; i < 10; ++i) {
        int value = id * 100 + i;

        // push() returns false if the bounded stack is full.
        // We retry instead of dropping the value.
        while (!stack.push(value)) {
            // Busy-wait until space becomes available.
        }
    }
}

void consumer(int id) {
    int value;

    while (true) {
        // Check whether all 30 produced values were consumed.
        int current = consumed.load(std::memory_order_relaxed);

        if (current >= 30) {
            break;
        }

        // pop() returns false when the stack is temporarily empty.
        if (stack.pop(value)) {
            // Atomically claim one consumed item.
            // fetch_add() prevents two consumers from using
            // the same counter value.
            int previous = consumed.fetch_add(1,std::memory_order_relaxed);

            // Only print values that belong to the expected
            // 30-item workload.
            if (previous < 30) {
                std::cout << "consumer " << id << " popped: " << value << '\n';
            }
        }
    }
}

int main() {
    // Three independent producers push into the same stack.
    std::thread producer1(producer, 1);
    std::thread producer2(producer, 2);
    std::thread producer3(producer, 3);

    producer1.join();
    producer2.join();
    producer3.join();

    // Two consumers concurrently pop from the same stack.
    std::thread consumer1(consumer, 1);
    std::thread consumer2(consumer, 2);

    consumer1.join();
    consumer2.join();
}