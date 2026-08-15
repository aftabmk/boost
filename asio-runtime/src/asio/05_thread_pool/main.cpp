#include <boost/asio.hpp>

#include <iostream>
#include <thread>

int main()
{
    // Creates a thread pool with four worker threads managed by Asio.
    boost::asio::thread_pool pool(4);

    // Submits eight independent tasks to the thread pool.
    for (int i = 0; i < 8; ++i)
    {
        boost::asio::post(pool, [i] {
            // The task executes on one of the pool's worker threads.
            std::cout << "Task " << i
                      << " executed on thread "
                      << std::this_thread::get_id()
                      << '\n';
        });
    }

    // Waits for all submitted tasks to finish before continuing.
    pool.join();

    return 0;
}