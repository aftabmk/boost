#include <boost/lockfree/queue.hpp>

#include <iostream>

int main() {
    // Fixed-capacity queue; storage is allocated up front.
    boost::lockfree::queue<int,boost::lockfree::capacity<8>> queue;

    // Add elements until the queue is full.
    for (int i = 0; i < 8; ++i) {
        bool success = queue.push(i);

        std::cout << "push " << i<< " -> " << std::boolalpha<< success << '\n';
    }

    int value;

    // Remove every available element.
    while (queue.pop(value)) {
        std::cout << "pop -> "<< value << '\n';
    }
}