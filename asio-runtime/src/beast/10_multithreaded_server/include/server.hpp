#pragma once

#include "listener.hpp"
#include "thread_pool.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>

namespace net = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// SERVER
// =============================================================================

class Server
{
private:

    net::io_context ioc;

    std::shared_ptr<Listener> listener;

    std::unique_ptr<ThreadPool> thread_pool;


public:

    Server(
        unsigned short port,
        unsigned int thread_count)
    {
        listener =
            std::make_shared<Listener>(
                ioc,
                tcp::endpoint(
                    tcp::v4(),
                    port
                )
            );


        thread_pool =
            std::make_unique<ThreadPool>(
                ioc,
                thread_count
            );
    }


    void run()
    {
        listener->run();


        std::cout
            << "Server listening on "
               "http://127.0.0.1:8080\n";


        thread_pool->join();
    }
};