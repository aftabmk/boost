#include <boost/lockfree/queue.hpp>

#include <iostream>

int main() {
    // Small capacity makes the full-queue condition easy to observe.
    boost::lockfree::queue<int,boost::lockfree::capacity<4>> queue;

    for (int i = 0; i < 10; ++i) {
        bool success = queue.push(i);

        if (success) {
            std::cout << "pushed: " << i << '\n';
        } 
		else {
            // push() fails immediately when no slot is available.
            std::cout << "queue full, rejected: "<< i << '\n';
        }
    }

    int value;

    while (queue.pop(value)) {
        std::cout << "popped: "<< value << '\n';
    }
}