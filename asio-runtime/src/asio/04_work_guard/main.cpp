#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    // Create the execution context that manages and dispatches asynchronous work.
    boost::asio::io_context io;

    // Create a work guard so io.run() remains active even when the queue is temporarily empty.
    auto work_guard = boost::asio::make_work_guard(io);

    // Start a worker thread that continuously processes work from the io_context.
    std::thread worker([&io] {
        // run() normally returns when no work remains; the work guard prevents that.
        io.run();
    });

    // Give the worker time to enter io.run() and wait for work.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Submit work after the worker is already waiting inside io.run().
    boost::asio::post(io, [] {
        std::cout << "Task executed\n";
    });

    // Allow the queued work to be processed before shutting down.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Remove the artificial work that keeps io.run() alive.
    work_guard.reset();

    // Wait for io.run() to finish after all queued work has been processed.
    worker.join();

    return 0;
}