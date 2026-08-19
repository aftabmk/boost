#include "include/listener.hpp"
#include "include/application.hpp"

#include <boost/asio/io_context.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

namespace net = boost::asio;

using tcp = net::ip::tcp;

using namespace beast_server;


int main()
{
    try
    {
        net::io_context ioc;


        Application application;


        auto listener =
            std::make_shared<Listener>(
                ioc,

                tcp::endpoint(
                    tcp::v4(),
                    8080
                ),

                application
            );


        listener->run();


        const unsigned int thread_count =
            std::max(
                1u,
                std::thread::hardware_concurrency()
            );


        std::cout
            << "Server listening on "
               "http://127.0.0.1:8080\n";

        std::cout
            << "Worker threads: "
            << thread_count
            << '\n';


        std::vector<std::thread> workers;

        workers.reserve(thread_count);


        for (unsigned int i = 0;
             i < thread_count;
             ++i)
        {
            workers.emplace_back(
                [&ioc]()
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
    catch (const beast_server::beast::system_error& e)
    {
        std::cerr
            << "Beast error: "
            << e.code().message()
            << '\n';

        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Fatal error: "
            << e.what()
            << '\n';

        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}