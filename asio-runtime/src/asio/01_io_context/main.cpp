#include <boost/asio.hpp>
#include <iostream>

int main()
{
    boost::asio::io_context io;

    boost::asio::post(io, [] {
        std::cout << "Task executed\n";
    });

    io.run();

    return 0;
}