#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

constexpr int item_count = 1'000'000;

std::queue<int> queue;

std::mutex mutex;
std::condition_variable cv;

bool finished = false;

void producer() {
    for (int i = 0; i < item_count; ++i) {
        {
            // Protect the standard queue from concurrent access.
            std::lock_guard<std::mutex> lock(mutex);

            queue.push(i);
        }

        // Wake the consumer after adding an item.
        cv.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(mutex);

        // Tell the consumer that no more items will arrive.
        finished = true;
    }

    cv.notify_one();
}

void consumer() {
    int count = 0;

    while (count < item_count) {
        std::unique_lock<std::mutex> lock(mutex);

        // Sleep instead of continuously spinning when empty.
        cv.wait(lock, [] {
            return !queue.empty() || finished;
        });

        if (!queue.empty()) {
            int value = queue.front();
            queue.pop();

            ++count;
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

    std::cout << "Mutex queue\n";
    std::cout << "items: " << item_count << '\n'; 
    std::cout << "time: "<< elapsed.count()<< " ms\n";
}

// Mutex queue
// items: 1000000
// time: 95 ms