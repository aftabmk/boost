#include <boost/asio.hpp>

#include <iostream>
#include <thread>

int main()
{
    // Creates the execution context shared by the worker threads.
    boost::asio::io_context io;

    // Creates a strand that guarantees handlers submitted through it
    // never execute concurrently with each other.
    auto strand = boost::asio::make_strand(io);

    // Schedules multiple handlers through the same strand.
    for (int i = 0; i < 8; ++i)
    {
        boost::asio::post(strand, [i] {
            // These handlers execute sequentially, even when multiple
            // worker threads are processing the same io_context.
            std::cout << "Task " << i
                      << " executed on thread "
                      << std::this_thread::get_id()
                      << '\n';
        });
    }

    // Starts multiple threads that concurrently process the io_context.
    std::jthread worker1([&io] {
        // Processes ready handlers from the shared execution context.
        io.run();
    });

    std::jthread worker2([&io] {
        // Processes ready handlers from the shared execution context.
        io.run();
    });

    std::jthread worker3([&io] {
        // Processes ready handlers from the shared execution context.
        io.run();
    });

    std::jthread worker4([&io] {
        // Processes ready handlers from the shared execution context.
        io.run();
    });

    return 0;
}

// Note
// Avoid need for mutex locks in spmc condition, 
// each worker does unique job without needing blocking other thread, 
// using user-space queue scheduling