#pragma once

#include <boost/asio/io_context.hpp>

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

namespace net = boost::asio;


// =============================================================================
// THREAD POOL
// =============================================================================
//
// Multiple OS threads execute the SAME io_context.
//
//
//
//             io_context
//                  │
//        ┌─────────┼─────────┐
//        │         │         │
//      thread    thread    thread
//        │         │         │
//        └─────────┼─────────┘
//                  │
//             Asio handlers
//
// The io_context is the scheduler.
//
// The threads are simply workers executing ready handlers.
//
// =============================================================================

class ThreadPool
{
private:

    net::io_context& ioc;

    std::vector<std::thread> workers;


public:

    ThreadPool(
        net::io_context& context,
        unsigned int thread_count)
        : ioc(context)
    {
        thread_count =
            std::max(
                1u,
                thread_count
            );


        workers.reserve(
            thread_count
        );


        for (unsigned int i = 0;
             i < thread_count;
             ++i)
        {
            workers.emplace_back(
                [this]()
                {
                    ioc.run();
                }
            );
        }
    }


    ~ThreadPool()
    {
        join();
    }


    ThreadPool(const ThreadPool&) = delete;

    ThreadPool& operator=(
        const ThreadPool&
    ) = delete;


    void join()
    {
        for (auto& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }


    std::size_t size() const
    {
        return workers.size();
    }
};