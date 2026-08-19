#include "include/server.hpp"

#include <boost/beast/core.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <thread>


int main()
{
    try
    {
        const unsigned int thread_count =
            std::max(
                1u,
                std::thread::hardware_concurrency()
            );


        std::cout
            << "Worker threads: "
            << thread_count
            << '\n';


        Server server(8080,thread_count);

        server.run();
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