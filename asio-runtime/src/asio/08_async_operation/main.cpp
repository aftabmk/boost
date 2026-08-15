#include <boost/asio.hpp>

#include <iostream>

int main()
{
    // Creates the execution context used to dispatch asynchronous operations.
    boost::asio::io_context io;

    // Creates a timer associated with the io_context.
    boost::asio::steady_timer timer(
        io,
        std::chrono::seconds(1)
    );

    // Starts an asynchronous timer operation without blocking the current thread.
    timer.async_wait(
        [](const boost::system::error_code& error)
        {
            // A successful asynchronous operation produces an empty error_code.
            if (!error)
            {
                std::cout << "Async operation completed\n";
                return;
            }

            // An error_code describes why the asynchronous operation failed or was cancelled.
            std::cout << "Async operation failed: "
                      << error.message()
                      << '\n';
        }
    );

    // Processes the completion handler when the asynchronous operation finishes.
    io.run();

    return 0;
}
// Note
// simple async await with timout