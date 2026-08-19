#include "include/application.hpp"
#include "include/listener.hpp"
#include "thread_pool.hpp"

#include <boost/beast/core.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>


int main()
{
    try
    {
        // =====================================================================
        // IO CONTEXT
        // =====================================================================

        boost::asio::io_context ioc;


        // =====================================================================
        // APPLICATION
        // =====================================================================

        Application application;


        // =====================================================================
        // LISTENER
        // =====================================================================

        auto listener =
            std::make_shared<Listener>(
                ioc,
                boost::asio::ip::tcp::endpoint(
                    boost::asio::ip::tcp::v4(),
                    8080
                ),
                application
            );


        listener->run();


        // =====================================================================
        // THREAD POOL
        // =====================================================================

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


        ThreadPool pool(ioc);

        pool.start(thread_count);

        pool.join();
    }
    catch (const boost::beast::system_error& e)
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