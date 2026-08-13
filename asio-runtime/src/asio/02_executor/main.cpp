#include <boost/asio.hpp>

#include <iostream>
#include <thread>

int main()
{
    // io_context owns the Asio execution context and dispatches queued asynchronous work.
    boost::asio::io_context io;

    // The executor is the scheduling interface associated with this io_context.
    auto executor = io.get_executor();

    // post() submits work to the executor; the lambda runs when io.run() processes it.
    boost::asio::post(executor, [] {
        std::cout << "Task executed on thread: "
                  << std::this_thread::get_id()
                  << '\n';
    });

    // run() makes the current thread execute ready work until the context has no work left.
    io.run();

    return 0;
}