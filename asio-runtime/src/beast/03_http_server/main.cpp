#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

using tcp = net::ip::tcp;

// -----------------------------------------------------------------------------
// Handle one HTTP connection.
//
// This function represents the lifetime of ONE client connection:
//
//     accept
//       ↓
//     read request
//       ↓
//     create response
//       ↓
//     write response
//       ↓
//     close
// -----------------------------------------------------------------------------
void handle_connection(tcp::socket socket)
{
    try
    {
        // ---------------------------------------------------------------------
        // 1. BUFFER FOR INCOMING HTTP DATA
        //
        // TCP is a byte stream. A complete HTTP request might arrive through
        // several TCP reads.
        //
        // flat_buffer temporarily stores those bytes while Beast parses them.
        // ---------------------------------------------------------------------

        beast::flat_buffer buffer;


        // ---------------------------------------------------------------------
        // 2. HTTP REQUEST OBJECT
        //
        // Beast will parse the bytes received from the client into this object.
        //
        // string_body means the request body is stored in std::string.
        // ---------------------------------------------------------------------

        http::request<http::string_body> request;


        // ---------------------------------------------------------------------
        // 3. READ THE HTTP REQUEST
        //
        // Beast performs:
        //
        //     TCP bytes
        //          ↓
        //     HTTP parser
        //          ↓
        //     request object
        //
        // This call does not return until the complete HTTP request has
        // been received and parsed.
        // ---------------------------------------------------------------------

        http::read(socket, buffer, request);


        // ---------------------------------------------------------------------
        // 4. INSPECT THE REQUEST
        // ---------------------------------------------------------------------

        std::cout << "\n===== REQUEST =====\n";

        std::cout << "Method : "<< request.method_string()<< '\n';

        std::cout << "Target : "<< request.target()<< '\n';

        std::cout << "Version: "<< request.version()<< '\n';


        // ---------------------------------------------------------------------
        // 5. APPLICATION LOGIC
        //
        // In a real application this is where routing/business logic would
        // happen.
        //
        // Example:
        //
        //     /           -> homepage
        //     /users      -> user API
        //     /products   -> product API
        //
        // For this tutorial we simply return a response based on the target.
        // ---------------------------------------------------------------------

        http::response<http::string_body> response;


        // HTTP version should normally match the request.
        response.version(request.version());


        // Keep the connection alive if the client requested it.
        response.keep_alive(request.keep_alive());


        if (request.target() == "/") {
            // -------------------------------------------------------------
            // HTTP 200 OK
            // -------------------------------------------------------------

            response.result(http::status::ok);

            response.set(
                http::field::content_type,
                "text/plain"
            );

            response.body() =
                "Hello from Boost.Beast HTTP Server!\n";
        }
        else if (request.target() == "/hello") {
            response.result(http::status::ok);

            response.set(
                http::field::content_type,
                "text/plain"
            );

            response.body() =
                "Hello, client!\n";
        }
        else {
            // -------------------------------------------------------------
            // Unknown route.
            //
            // HTTP 404 means the requested resource was not found.
            // -------------------------------------------------------------

            response.result(http::status::not_found);

            response.set(http::field::content_type,"text/plain");

            response.body() = "404 - Resource not found\n";
        }


        // ---------------------------------------------------------------------
        // 6. PREPARE THE RESPONSE BODY
        //
        // Beast calculates body-related headers such as Content-Length.
        //
        // Without this, the response object does not automatically know the
        // final size of the string body.
        // ---------------------------------------------------------------------

        response.prepare_payload();


        // ---------------------------------------------------------------------
        // 7. WRITE THE HTTP RESPONSE
        //
        // Beast performs:
        //
        //     response object
        //          ↓
        //     HTTP serializer
        //          ↓
        //     TCP bytes
        //
        // The client receives those bytes and its HTTP parser reconstructs
        // the response object.
        // ---------------------------------------------------------------------

        http::write(socket, response);


        // ---------------------------------------------------------------------
        // 8. CLOSE CONNECTION WHEN KEEP-ALIVE IS NOT REQUESTED
        //
        // HTTP/1.1 normally uses persistent connections.
        //
        // If keep_alive() is false, this connection should be closed.
        // ---------------------------------------------------------------------

        if (!response.keep_alive()) {
            beast::error_code ec;

            socket.shutdown(tcp::socket::shutdown_send,ec);

            // An already-closed connection is not a serious server failure.
            if (ec){
                std::cerr << "Shutdown error: "<< ec.message()<< '\n';
            }
        }
    }
    catch (const std::exception& e){
        // ---------------------------------------------------------------------
        // A client can disconnect, send malformed HTTP, reset the TCP
        // connection, etc.
        //
        // For this synchronous tutorial we simply report the error.
        // Later we will handle these errors explicitly using error_code.
        // ---------------------------------------------------------------------

        std::cerr << "Connection error: "<< e.what()<< '\n';
    }
}


int main(){
    try {
        // ---------------------------------------------------------------------
        // 1. ASIO EXECUTION CONTEXT
        // ---------------------------------------------------------------------

        net::io_context ioc;


        // ---------------------------------------------------------------------
        // 2. CREATE TCP ACCEPTOR
        //
        // The acceptor listens for incoming TCP connections.
        //
        // Port 8080 is used so that this server can run as a normal
        // unprivileged development process.
        // ---------------------------------------------------------------------

        tcp::acceptor acceptor{
            ioc,
            tcp::endpoint{tcp::v4(),8080}
        };


        std::cout << "HTTP server listening on http://127.0.0.1:8080\n";


        // ---------------------------------------------------------------------
        // 3. ACCEPT CLIENT CONNECTIONS
        //
        // This loop handles clients one at a time.
        //
        // IMPORTANT:
        //
        // This is intentionally NOT a production architecture.
        //
        // While handle_connection() is processing one client, the acceptor
        // cannot process another client.
        //
        // We will solve this with asynchronous sessions later.
        // ---------------------------------------------------------------------

        for (;;) {
            // Create a socket that will represent the accepted client.
            tcp::socket socket{ioc};

            // Wait for a client to establish a TCP connection.
            acceptor.accept(socket);

            std::cout << "\nClient connected\n";


            // -------------------------------------------------------------
            // Process this connection.
            // -------------------------------------------------------------

            handle_connection(std::move(socket));


            std::cout << "Client disconnected\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: "<< e.what()<< '\n';

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}