#pragma once

#include <boost/asio/io_context.hpp>

#include <algorithm>
#include <cstddef>
#include <thread>
#include <vector>

namespace net = boost::asio;


// =============================================================================
// THREAD POOL
// =============================================================================

class ThreadPool
{
private:

    net::io_context& ioc;

    std::vector<std::thread> workers;


public:

    explicit ThreadPool(
        net::io_context& context)
        : ioc(context)
    {
    }


    void start(
        unsigned int thread_count)
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


    void join()
    {
        for (auto& worker : workers)
        {
            if (worker.joinable())
                worker.join();
        }
    }


    std::size_t size() const
    {
        return workers.size();
    }
};