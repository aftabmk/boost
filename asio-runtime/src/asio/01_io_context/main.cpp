#include <boost/asio.hpp>
#include <iostream>

int main()
{
    // Creates the Asio execution context that owns and dispatches asynchronous work.
    boost::asio::io_context io;

    // Submits a task to the io_context; the task executes when io.run() processes it.
    boost::asio::post(io, [] {
        std::cout << "Task executed\n";
    });

    // Starts processing queued handlers and returns when no work remains.
    io.run();

    return 0;
}