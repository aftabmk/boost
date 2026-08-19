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
// Listener:
//
//     TCP accept
//          ↓
//       Session
//
// It does NOT know anything about HTTP requests.
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
        tcp::endpoint endpoint
    )
        : ioc(context),
          acceptor(context)
    {
        beast::error_code ec;


        // ---------------------------------------------------------------------
        // Open
        // ---------------------------------------------------------------------

        acceptor.open(
            endpoint.protocol(),
            ec
        );

        if (ec)
            throw beast::system_error(ec);


        // ---------------------------------------------------------------------
        // Reuse address
        // ---------------------------------------------------------------------

        acceptor.set_option(
            net::socket_base::reuse_address(true),
            ec
        );

        if (ec)
            throw beast::system_error(ec);


        // ---------------------------------------------------------------------
        // Bind
        // ---------------------------------------------------------------------

        acceptor.bind(
            endpoint,
            ec
        );

        if (ec)
            throw beast::system_error(ec);


        // ---------------------------------------------------------------------
        // Listen
        // ---------------------------------------------------------------------

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

    void accept()
    {
        acceptor.async_accept(

            beast::bind_front_handler(
                &Listener::on_accept,
                shared_from_this()
            )
        );
    }


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
            << "[Listener] New client accepted\n";


        Session::create(
            std::move(socket)
        )->run();


        // Immediately wait for another client.

        accept();
    }
};