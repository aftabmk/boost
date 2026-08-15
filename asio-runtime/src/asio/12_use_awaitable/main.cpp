#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

using namespace boost::asio;

// An awaitable coroutine that uses Asio's use_awaitable completion token.
awaitable<void> task() {
    // Get the executor associated with the currently running coroutine.
    auto executor = co_await this_coro::executor;

    // Create a timer associated with the coroutine's executor.
    steady_timer timer(executor,std::chrono::seconds(1));

    // use_awaitable converts async_wait() into an awaitable operation.
    // co_await suspends this coroutine until the timer completes.
    co_await timer.async_wait(use_awaitable);

    // Execution resumes here after the timer expires.
    std::cout << "Timer completed\n";

    co_return;
}

int main()
{
    // Creates the Asio execution context used to execute the coroutine.
    io_context io;

    // Starts the coroutine and schedules it on the io_context.
    co_spawn(io,task(),detached);

    // Runs the execution context until the coroutine has completed.
    io.run();

    return 0;
}