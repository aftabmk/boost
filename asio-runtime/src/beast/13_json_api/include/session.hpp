#pragma once

#include "application.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <utility>


namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

using tcp = net::ip::tcp;


class Session
    : public std::enable_shared_from_this<Session>
{
private:

    beast::tcp_stream stream;

    beast::flat_buffer buffer;

    Request request;


    net::strand<
        net::any_io_executor
    > strand;


    Application& application;


    Session(
        tcp::socket&& socket,
        Application& app)
        : stream(
            std::move(socket)
        ),
          strand(
              stream.get_executor()
          ),
          application(app)
    {
    }


public:

    static std::shared_ptr<Session> create(
        tcp::socket&& socket,
        Application& application)
    {
        return std::shared_ptr<Session>(
            new Session(
                std::move(socket),
                application
            )
        );
    }


    // =========================================================================
    // RUN
    // =========================================================================

    void run()
    {
        net::dispatch(
            strand,

            beast::bind_front_handler(
                &Session::do_read,
                shared_from_this()
            )
        );
    }


private:

    // =========================================================================
    // READ
    // =========================================================================

    void do_read()
    {
        stream.expires_after(
            std::chrono::seconds(30)
        );


        request = {};


        http::async_read(
            stream,
            buffer,
            request,

            net::bind_executor(
                strand,

                beast::bind_front_handler(
                    &Session::on_read,
                    shared_from_this()
                )
            )
        );
    }


    // =========================================================================
    // READ COMPLETION
    // =========================================================================

    void on_read(
        beast::error_code ec,
        std::size_t bytes)
    {
        (void)bytes;


        if (ec)
        {
            if (ec == http::error::end_of_stream ||
                ec == net::error::eof)
            {
                close();
                return;
            }


            if (ec == beast::error::timeout)
            {
                std::cerr
                    << "[Session] Read timeout\n";

                close();
                return;
            }


            if (ec == net::error::connection_reset)
            {
                close();
                return;
            }


            std::cerr
                << "[Session] Read error: "
                << ec.message()
                << '\n';


            close();

            return;
        }


        // ---------------------------------------------------------------------
        // Application layer.
        // ---------------------------------------------------------------------

        Response response =
            application.handle(
                request
            );


        write_response(
            std::move(response)
        );
    }


    // =========================================================================
    // WRITE
    // =========================================================================

    void write_response(
        Response response)
    {
        auto response_ptr =
            std::make_shared<Response>(
                std::move(response)
            );


        stream.expires_after(
            std::chrono::seconds(30)
        );


        http::async_write(
            stream,
            *response_ptr,

            net::bind_executor(
                strand,

                beast::bind_front_handler(
                    &Session::on_write,
                    shared_from_this(),
                    response_ptr
                )
            )
        );
    }


    // =========================================================================
    // WRITE COMPLETION
    // =========================================================================

    void on_write(
        std::shared_ptr<Response> response,
        beast::error_code ec,
        std::size_t bytes)
    {
        (void)bytes;


        if (ec)
        {
            if (ec == net::error::connection_reset)
            {
                close();
                return;
            }


            if (ec == beast::error::timeout)
            {
                std::cerr
                    << "[Session] Write timeout\n";

                close();
                return;
            }


            std::cerr
                << "[Session] Write error: "
                << ec.message()
                << '\n';


            close();

            return;
        }


        if (!response->keep_alive())
        {
            close();
            return;
        }


        // ---------------------------------------------------------------------
        // HTTP keep-alive.
        // ---------------------------------------------------------------------

        do_read();
    }


    // =========================================================================
    // CLOSE
    // =========================================================================

    void close()
    {
        beast::error_code ec;


        stream.socket().shutdown(
            tcp::socket::shutdown_send,
            ec
        );


        if (ec &&
            ec != net::error::not_connected &&
            ec != net::error::eof)
        {
            std::cerr
                << "[Session] Shutdown error: "
                << ec.message()
                << '\n';
        }
    }
};