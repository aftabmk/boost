#pragma once

#include "application.hpp"
#include "listener.hpp"

#include <boost/asio/io_context.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>


namespace net = boost::asio;


class Server
{
private:

    net::io_context ioc;

    Application application;

    std::shared_ptr<Listener> listener;

    std::vector<std::thread> workers;

    unsigned short port;

    unsigned int thread_count;


public:

    explicit Server(
        unsigned short server_port = 8080)
        : port(server_port),
          thread_count(
              std::max(
                  1u,
                  std::thread::hardware_concurrency()
              )
          )
    {
        listener =
            std::make_shared<Listener>(
                ioc,

                net::ip::tcp::endpoint(
                    net::ip::tcp::v4(),
                    port
                ),

                application
            );
    }


    // =========================================================================
    // RUN
    // =========================================================================

    void run()
    {
        listener->run();


        std::cout
            << "Server listening on "
               "http://127.0.0.1:"
            << port
            << '\n';


        std::cout
            << "Worker threads: "
            << thread_count
            << '\n';


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


        for (auto& worker : workers)
        {
            worker.join();
        }
    }
};