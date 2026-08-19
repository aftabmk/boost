#pragma once

#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <iostream>
#include <memory>

namespace beast = boost::beast;
namespace net   = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// LISTENER
// =============================================================================
//
// Accepts TCP connections and creates Sessions.
//
// Listener does not manage worker threads.
// The io_context is executed by ThreadPool.
//
// =============================================================================

class Listener
    : public std::enable_shared_from_this<Listener>
{
private:

    net::io_context& ioc;

    tcp::acceptor acceptor;


public:

    Listener(
        net::io_context& context,
        tcp::endpoint endpoint)
        : ioc(context),
          acceptor(context)
    {
        beast::error_code ec;


        acceptor.open(
            endpoint.protocol(),
            ec
        );

        if (ec)
            throw beast::system_error(ec);


        acceptor.set_option(
            net::socket_base::reuse_address(true),
            ec
        );

        if (ec)
            throw beast::system_error(ec);


        acceptor.bind(
            endpoint,
            ec
        );

        if (ec)
            throw beast::system_error(ec);


        acceptor.listen(
            net::socket_base::max_listen_connections,
            ec
        );

        if (ec)
            throw beast::system_error(ec);
    }


    void run()
    {
        accept();
    }


private:

    // =========================================================================
    // ACCEPT
    // =========================================================================

    void accept()
    {
        acceptor.async_accept(

            beast::bind_front_handler(
                &Listener::on_accept,
                shared_from_this()
            )
        );
    }


    // =========================================================================
    // ACCEPT COMPLETION
    // =========================================================================

    void on_accept(
        beast::error_code ec,
        tcp::socket socket)
    {
        if (ec)
        {
            std::cerr
                << "[Listener] Accept error: "
                << ec.message()
                << '\n';

            accept();

            return;
        }


        std::cout
            << "[Listener] Connection accepted\n";


        Session::create(
            std::move(socket)
        )->run();


        // Continue accepting clients.

        accept();
    }
};