#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// SESSION
// =============================================================================
//
// One Session = one client connection.
//
// The Session does NOT own a thread.
//
// Any thread executing io_context::run() may execute the Session's handlers.
//
// The strand guarantees that handlers belonging to this Session execute
// sequentially.
//
// =============================================================================

class Session
    : public std::enable_shared_from_this<Session>
{
private:

    beast::tcp_stream stream;

    beast::flat_buffer buffer;

    http::request<http::string_body> request;

    net::strand<net::any_io_executor> strand;


    explicit Session(tcp::socket&& socket)
        : stream(std::move(socket)),
          strand(stream.get_executor())
    {
    }


public:

    static std::shared_ptr<Session> create(
        tcp::socket&& socket)
    {
        return std::shared_ptr<Session>(
            new Session(std::move(socket))
        );
    }


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
                    << "["
                    << std::this_thread::get_id()
                    << "] Read timeout\n";

                close();
                return;
            }


            if (ec == net::error::connection_reset)
            {
                close();
                return;
            }


            std::cerr
                << "["
                << std::this_thread::get_id()
                << "] Read error: "
                << ec.message()
                << '\n';

            close();

            return;
        }


        std::cout
            << "["
            << std::this_thread::get_id()
            << "] "
            << request.method_string()
            << " "
            << request.target()
            << '\n';


        write_response();
    }


    // =========================================================================
    // CREATE RESPONSE
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
        // /
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
                "Hello from multi-threaded Beast!\n";
        }


        // ---------------------------------------------------------------------
        // /thread
        // ---------------------------------------------------------------------

        else if (request.target() == "/thread")
        {
            response->result(
                http::status::ok
            );

            response->set(
                http::field::content_type,
                "text/plain"
            );

            response->body() =
                "Request processed by worker thread: " +
                std::to_string(
                    std::hash<std::thread::id>{}(
                        std::this_thread::get_id()
                    )
                ) +
                "\n";
        }


        // ---------------------------------------------------------------------
        // 404
        // ---------------------------------------------------------------------

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
                "404 - Not Found\n";
        }


        response->prepare_payload();


        stream.expires_after(
            std::chrono::seconds(30)
        );


        http::async_write(
            stream,
            *response,

            net::bind_executor(
                strand,

                beast::bind_front_handler(
                    &Session::on_write,
                    shared_from_this(),
                    response
                )
            )
        );
    }


    // =========================================================================
    // WRITE COMPLETION
    // =========================================================================

    void on_write(std::shared_ptr<http::response<http::string_body>> response,
        beast::error_code ec,std::size_t bytes) {
        (void)bytes;


        if (ec) {
            if (ec == net::error::connection_reset) {
                close();
                return;
            }


            if (ec == beast::error::timeout){
                std::cerr<< "["<< std::this_thread::get_id()<< "] Write timeout\n";

                close();
                return;
            }


            std::cerr<< "["<< std::this_thread::get_id()<< "] Write error: "<< ec.message()<< '\n';
            close();
            return;
        }


        if (!response->keep_alive()){
            close();
            return;
        }


        do_read();
    }


    // =========================================================================
    // CLOSE
    // =========================================================================

    void close(){
        beast::error_code ec;


        stream.socket().shutdown(
            tcp::socket::shutdown_send,
            ec
        );


        if (ec &&
            ec != net::error::not_connected &&
            ec != net::error::eof) {
            std::cerr
                << "["
                << std::this_thread::get_id()
                << "] Shutdown error: "
                << ec.message()
                << '\n';
        }
    }
};