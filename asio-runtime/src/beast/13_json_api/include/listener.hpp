#pragma once

#include "session.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

#include <iostream>
#include <memory>
#include <utility>


namespace beast = boost::beast;
namespace net   = boost::asio;

using tcp = net::ip::tcp;


class Listener
    : public std::enable_shared_from_this<Listener>
{
private:

    net::io_context& ioc;

    tcp::acceptor acceptor;

    Application& application;


public:

    Listener(
        net::io_context& context,
        tcp::endpoint endpoint,
        Application& app)
        : ioc(context),
          acceptor(context),
          application(app)
    {
        beast::error_code ec;


        // ---------------------------------------------------------------------
        // OPEN
        // ---------------------------------------------------------------------

        acceptor.open(
            endpoint.protocol(),
            ec
        );


        if (ec)
            throw beast::system_error(ec);


        // ---------------------------------------------------------------------
        // REUSE ADDRESS
        // ---------------------------------------------------------------------

        acceptor.set_option(
            net::socket_base::reuse_address(true),
            ec
        );


        if (ec)
            throw beast::system_error(ec);


        // ---------------------------------------------------------------------
        // BIND
        // ---------------------------------------------------------------------

        acceptor.bind(
            endpoint,
            ec
        );


        if (ec)
            throw beast::system_error(ec);


        // ---------------------------------------------------------------------
        // LISTEN
        // ---------------------------------------------------------------------

        acceptor.listen(
            net::socket_base::max_listen_connections,
            ec
        );


        if (ec)
            throw beast::system_error(ec);
    }


    // =========================================================================
    // RUN
    // =========================================================================

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


            // Continue accepting.
            accept();

            return;
        }


        std::cout
            << "[Listener] Client connected\n";


        // ---------------------------------------------------------------------
        // Session takes ownership of socket.
        // ---------------------------------------------------------------------

        Session::create(
            std::move(socket),
            application
        )->run();


        // ---------------------------------------------------------------------
        // Immediately accept another client.
        // ---------------------------------------------------------------------

        accept();
    }
};