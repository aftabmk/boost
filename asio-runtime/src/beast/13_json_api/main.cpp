#include "include/server.hpp"

#include <boost/beast/core.hpp>

#include <cstdlib>
#include <iostream>


namespace beast = boost::beast;


int main()
{
    try
    {
        Server server(
            8080
        );


        server.run();
    }
    catch (const beast::system_error& e)
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