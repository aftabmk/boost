#include <boost/asio.hpp>
#include <iostream>

// Runs on the io_context's single thread. co_await suspends
// this coroutine without blocking the thread — the thread is
// free to resume other coroutines while this one waits.
boost::asio::awaitable<void> task(int id, boost::asio::steady_timer::duration delay)
{
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
    timer.expires_after(delay);
    co_await timer.async_wait(boost::asio::use_awaitable);  // suspend, don't block

    std::cout << "Task " << id << " done\n";
}

int main()
{
    boost::asio::io_context io;  // single-threaded event loop

    // Schedule both tasks; neither runs until io.run() starts.
    boost::asio::co_spawn(io, task(1, std::chrono::seconds(2)), boost::asio::detached);
    boost::asio::co_spawn(io, task(2, std::chrono::seconds(1)), boost::asio::detached);

    io.run();  // drives both coroutines on one thread; task 2 finishes first

    return 0;
}