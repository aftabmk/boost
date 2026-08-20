#include <atomic>
#include <iostream>

int main() {
    std::atomic<int> value{10};

    int expected = 10;
    int desired = 20;

    // Change 10 → 20 only if value is still 10.
    bool success = value.compare_exchange_strong(
        expected,
        desired,
        std::memory_order_acq_rel,
        std::memory_order_acquire
    );

    std::cout << "success = " << success << '\n';
    std::cout << "value   = " << value.load() << '\n';
    std::cout << "expected = " << expected << '\n';
}