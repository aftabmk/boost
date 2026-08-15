#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

using namespace boost::asio;

// An awaitable coroutine that performs multiple asynchronous operations.
awaitable<void> task() {
    // Get the executor associated with the currently running coroutine.
    auto executor = co_await this_coro::executor;

    // Create the first asynchronous timer.
    steady_timer first_timer(executor,std::chrono::seconds(1));

    // Suspend the coroutine until the first timer completes.
    // The worker thread is free to execute other Asio work while suspended.
    co_await first_timer.async_wait(use_awaitable);

    std::cout << "First operation completed\n";

    // Create a second asynchronous timer after the first operation completes.
    steady_timer second_timer(executor,std::chrono::seconds(1));

    // Suspend again and resume when the second timer completes.
    co_await second_timer.async_wait(use_awaitable);

    std::cout << "Second operation completed\n";

    // Finish the coroutine.
    co_return;
}

int main() {
    // Creates the Asio execution context used by the coroutine.
    io_context io;

    // Starts the coroutine and schedules it on the io_context.
    co_spawn(io,task(),detached);

    // Runs the execution context until the coroutine has completed.
    io.run();

    return 0;
}