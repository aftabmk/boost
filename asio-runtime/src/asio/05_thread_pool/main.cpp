#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <vector>

int main()
{
    // Creates the Asio execution context that owns and dispatches asynchronous work.
    boost::asio::io_context io;

    // Schedules multiple independent tasks into the io_context.
    for (int i = 0; i < 8; ++i)
    {
        boost::asio::post(io, [i] {
            std::cout << "Task " << i
                      << " executed on thread "
                      << std::this_thread::get_id()
                      << '\n';
        });
    }

    // Creates multiple worker threads that concurrently process the same io_context.
    std::vector<std::jthread> workers;

    for (int i = 0; i < 4; ++i)
    {
        workers.emplace_back([&io] {
            // Each worker calls run(); Asio distributes ready handlers among these threads.
            io.run();
        });
    }

    // jthread automatically joins all worker threads when the vector is destroyed.
    return 0;
}