#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <vector>

int main()
{
    // Creates the execution context shared by all worker threads.
    boost::asio::io_context io;

    // Creates a work guard so io.run() stays alive even when the queue is temporarily empty.
    auto work_guard = boost::asio::make_work_guard(io);

    // Schedules work that can be processed by any worker thread.
    for (int i = 0; i < 8; ++i)
    {
        boost::asio::post(io, [i] {
            std::cout << "Task " << i
                      << " executed on thread "
                      << std::this_thread::get_id()
                      << '\n';
        });
    }

    // Starts multiple threads that concurrently process work from the same io_context.
    std::vector<std::jthread> workers;

    for (int i = 0; i < 4; ++i)
    {
        workers.emplace_back([&io] {
            // Each thread enters the Asio event loop and processes available handlers.
            io.run();
        });
    }

    // Releases the work guard so run() can finish after all queued work is processed.
    work_guard.reset();

    return 0;
}