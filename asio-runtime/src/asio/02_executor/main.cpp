#include <boost/asio.hpp>
#include <iostream>
#include <thread>

int main()
{
    boost::asio::io_context io;

    auto executor = io.get_executor();

    boost::asio::post(executor,[] 
		{
			std::cout << "Task executed on thread: "<< std::this_thread::get_id()<< '\n';
		}
    );

    io.run();

    return 0;
}