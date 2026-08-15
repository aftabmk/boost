#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

using namespace boost::asio;

// An awaitable represents a coroutine managed by Asio.
// The coroutine performs asynchronous work without blocking a thread.
awaitable<void> timer_task() {
    // Get the executor currently associated with this coroutine.
    auto executor = co_await this_coro::executor;

    // Create a timer using the coroutine's executor.
    steady_timer timer(executor,std::chrono::seconds(1));

    // Suspend the coroutine until the timer completes.
    co_await timer.async_wait(use_awaitable);

    // The coroutine resumes here after the timer expires.
    std::cout << "Coroutine completed\n";

    co_return;
}

int main() {
    // Creates the Asio execution context that runs asynchronous work.
    io_context io;

    // Starts timer_task() and schedules its execution on the io_context.
    co_spawn(io,timer_task(),detached);

    // Runs the io_context and allows the spawned coroutine to execute.
    io.run();

    return 0;
}