#include <boost/asio.hpp>

#include <iostream>
#include <thread>

int main()
{
    // Creates the execution context that manages and dispatches asynchronous work.
    boost::asio::io_context io;

    // post() schedules work for later execution; it never runs the handler inline.
    boost::asio::post(io, [] {
        std::cout << "Task 1 executed on thread: "
                  << std::this_thread::get_id()
                  << '\n';
    });

    // post() can schedule multiple independent tasks on the same execution context.
    boost::asio::post(io, [] {
        std::cout << "Task 2 executed on thread: "
                  << std::this_thread::get_id()
                  << '\n';
    });

    // post() returns immediately; the queued handlers execute only when io.run() is called.
    std::cout << "Tasks posted\n";

    // Runs queued handlers on the current thread until the context has no remaining work.
    io.run();

    return 0;
}