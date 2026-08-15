#include <boost/asio.hpp>

#include <chrono>
#include <iostream>

int main()
{
    // Creates the execution context that manages asynchronous operations.
    boost::asio::io_context io;

    // Creates a timer associated with the io_context and sets a 2-second expiry.
    boost::asio::steady_timer timer(
        io,
        std::chrono::seconds(2)
    );

    // Registers a callback that Asio invokes when the timer expires.
    timer.async_wait([](const boost::system::error_code& error)
    {
        // Checks whether the timer completed normally or was cancelled/failed.
        if (!error)
        {
            std::cout << "Timer expired\n";
            return;
        }

        // Reports the reason when the asynchronous timer operation did not complete normally.
        std::cout << "Timer error: "
                  << error.message()
                  << '\n';
    });

    // Runs the io_context until the asynchronous timer operation completes.
    io.run();

    return 0;
}