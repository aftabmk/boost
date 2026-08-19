#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// SESSION
// =============================================================================
//
// One Session = one connected HTTP client.
//
//     TCP connection
//          ↓
//       Session
//          ↓
//     read request
//          ↓
//     create response
//          ↓
//     write response
//          ↓
//     keep alive?
//        /    \
//      yes     no
//       ↓       ↓
//     read    close
//     next
//
// =============================================================================

class Session
    : public std::enable_shared_from_this<Session>
{
private:

    beast::tcp_stream stream;

    beast::flat_buffer buffer;

    http::request<http::string_body> request;


    explicit Session(tcp::socket&& socket)
        : stream(std::move(socket))
    {
    }


public:

    static std::shared_ptr<Session> create(tcp::socket&& socket)
    {
        return std::shared_ptr<Session>(
            new Session(std::move(socket))
        );
    }


    void run()
    {
        stream.expires_after(
            std::chrono::seconds(30)
        );

        read_request();
    }


private:

    // =========================================================================
    // READ
    // =========================================================================

    void read_request()
    {
        request = {};

        http::async_read(
            stream,
            buffer,
            request,

            beast::bind_front_handler(
                &Session::on_read,
                shared_from_this()
            )
        );
    }


    void on_read(
        beast::error_code ec,
        std::size_t bytes_read)
    {
        (void)bytes_read;

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
                    << "[Session] Request timeout\n";

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


        std::cout
            << "[Session] "
            << request.method_string()
            << " "
            << request.target()
            << '\n';


        write_response();
    }


    // =========================================================================
    // RESPONSE
    // =========================================================================

    void write_response()
    {
        auto response =
            std::make_shared<
                http::response<http::string_body>
            >();


        response->version(
            request.version()
        );


        response->keep_alive(
            request.keep_alive()
        );


        // ---------------------------------------------------------------------
        // Simple routing
        // ---------------------------------------------------------------------

        if (request.target() == "/")
        {
            response->result(
                http::status::ok
            );

            response->set(
                http::field::content_type,
                "text/plain"
            );

            response->body() =
                "Hello from asynchronous Beast server!\n";
        }
        else if (request.target() == "/hello")
        {
            response->result(
                http::status::ok
            );

            response->set(
                http::field::content_type,
                "text/plain"
            );

            response->body() =
                "Hello from /hello\n";
        }
        else
        {
            response->result(
                http::status::not_found
            );

            response->set(
                http::field::content_type,
                "text/plain"
            );

            response->body() =
                "404 - Resource not found\n";
        }


        response->prepare_payload();


        stream.expires_after(
            std::chrono::seconds(30)
        );


        http::async_write(
            stream,
            *response,

            beast::bind_front_handler(
                &Session::on_write,
                shared_from_this(),
                response
            )
        );
    }


    // =========================================================================
    // WRITE COMPLETION
    // =========================================================================

    void on_write(
        std::shared_ptr<
            http::response<http::string_body>
        > response,
        beast::error_code ec,
        std::size_t bytes_written)
    {
        (void)response;
        (void)bytes_written;


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


        stream.expires_after(
            std::chrono::seconds(30)
        );


        read_request();
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