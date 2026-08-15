#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

using namespace boost::asio;
using namespace std::chrono_literals;

// A coroutine that can be interrupted through Asio's cancellation mechanism.
awaitable<void> task() {
    // Get the executor associated with the running coroutine.
    auto executor = co_await this_coro::executor;

    // Create a timer that would normally keep the coroutine suspended for 10 seconds.
    steady_timer timer(executor, 10s);

    try {
        // Suspend the coroutine until the timer completes or is cancelled.
        co_await timer.async_wait(use_awaitable);

        std::cout << "Timer completed\n";
    }
    catch (const boost::system::system_error& error) {
        // Cancellation causes the awaited operation to complete with an error.
        std::cout << "Coroutine stopped: "
                  << error.code().message()
                  << '\n';
    }

    co_return;
}

int main() {
    // Creates the Asio execution context used by the coroutine.
    io_context io;

    // Creates a cancellation signal that can request cancellation of the coroutine.
    cancellation_signal cancel_signal;

    // Starts the coroutine and connects its cancellation slot to the signal.
    co_spawn(io,task(),
		bind_cancellation_slot(cancel_signal.slot(),detached)
    );

    // Schedule cancellation after one second instead of waiting for the 10-second timer.
    steady_timer cancel_timer(io, 1s);

    cancel_timer.async_wait(
        [&cancel_signal](const boost::system::error_code& error) {
            // Request cancellation only if the cancellation timer completed normally.
            if (!error)
                cancel_signal.emit(cancellation_type::all);
        }
    );

    // Runs the io_context until all remaining asynchronous work completes.
    io.run();

    return 0;
}