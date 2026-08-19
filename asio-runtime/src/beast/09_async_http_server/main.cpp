#include "server.hpp"

#include <boost/beast/core.hpp>

#include <cstdlib>
#include <iostream>


int main()
{
    try
    {
        Server server(8080);

        std::cout
            << "Async HTTP server listening on "
               "http://127.0.0.1:8080\n";

        server.run();
    }
    catch (const boost::beast::system_error& e)
    {
        std::cerr
            << "Beast system error: "
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