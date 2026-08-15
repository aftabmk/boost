#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

using namespace boost::asio;

// An awaitable represents a coroutine whose execution can suspend and resume
// when an asynchronous Asio operation completes.
awaitable<void> timer_task() {
    // Retrieves the executor currently associated with this coroutine.
    auto executor = co_await this_coro::executor;

    // Creates a timer using the coroutine's executor.
    steady_timer timer(executor,std::chrono::seconds(1));

    // Suspends the coroutine until the timer expires.
    co_await timer.async_wait(use_awaitable);

    // Execution resumes here after the asynchronous operation completes.
    std::cout << "Timer completed\n";

    co_return;
}

int main() {
    // Creates the Asio execution context used by the coroutine.
    io_context io;

    // Starts the coroutine and schedules it on the io_context.
    co_spawn(io,timer_task(),detached);

    // Runs the io_context until the coroutine has completed.
    io.run();

    return 0;
}