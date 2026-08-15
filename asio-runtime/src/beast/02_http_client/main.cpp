#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

using tcp = net::ip::tcp;

int main()
{
    try
    {
        // ---------------------------------------------------------------------
        // 1. BASIC CONNECTION INFORMATION
        //
        // We use example.com so the example does not depend on a local server.
        //
        // `host` is used both for DNS resolution and the HTTP Host header.
        // ---------------------------------------------------------------------

        const std::string host = "example.com";
        const std::string port = "80";
        const std::string target = "/";

        // HTTP/1.1 version.
        constexpr int version = 11;


        // ---------------------------------------------------------------------
        // 2. CREATE THE ASIO I/O CONTEXT
        //
        // io_context owns the execution context used by Asio operations.
        //
        // This example uses synchronous operations, so there is no
        // `io_context.run()` call yet.
        // ---------------------------------------------------------------------

        net::io_context ioc;


        // ---------------------------------------------------------------------
        // 3. RESOLVE THE HOSTNAME
        //
        // DNS:
        //
        //     "example.com"
        //          ↓
        //     IP address
        //
        // The resolver returns one or more endpoints that can be connected to.
        // ---------------------------------------------------------------------

        tcp::resolver resolver{ioc};

        auto const results = resolver.resolve(host, port);


        // ---------------------------------------------------------------------
        // 4. CREATE A TCP SOCKET
        //
        // The socket represents our TCP connection to the remote HTTP server.
        // ---------------------------------------------------------------------

        beast::tcp_stream stream{ioc};


        // ---------------------------------------------------------------------
        // 5. CONNECT TCP
        //
        // Beast's tcp_stream is built on top of Asio's TCP socket.
        //
        // `connect()` tries the resolved endpoints until one succeeds.
        // ---------------------------------------------------------------------

        stream.connect(results);

        std::cout << "Connected to " << host << '\n';


        // ---------------------------------------------------------------------
        // 6. CREATE THE HTTP REQUEST
        //
        // We use string_body because this GET request does not need a body.
        // ---------------------------------------------------------------------

        http::request<http::string_body> request{http::verb::get,target,version};

        // HTTP/1.1 requires the Host header.
        request.set(http::field::host, host);

        // Identify our client.
        request.set(http::field::user_agent,"Boost.Beast HTTP Client");


        // ---------------------------------------------------------------------
        // 7. SEND THE REQUEST
        //
        // `http::write()` serializes the C++ request object into HTTP bytes
        // and writes those bytes to the TCP stream.
        //
        // Internally the important conceptual operation is:
        //
        //     request object
        //          ↓
        //       serializer
        //          ↓
        //       TCP bytes
        // ---------------------------------------------------------------------

        http::write(stream, request);

        std::cout << "HTTP request sent\n";


        // ---------------------------------------------------------------------
        // 8. CREATE A BUFFER FOR THE RESPONSE
        //
        // The network does not necessarily give us the entire HTTP response
        // in one TCP read.
        //
        // `flat_buffer` stores bytes received from the socket while Beast's
        // HTTP parser processes them.
        // ---------------------------------------------------------------------

        beast::flat_buffer buffer;


        // ---------------------------------------------------------------------
        // 9. CREATE THE RESPONSE OBJECT
        //
        // Beast will parse the incoming HTTP bytes into this object.
        // ---------------------------------------------------------------------

        http::response<http::string_body> response;


        // ---------------------------------------------------------------------
        // 10. READ THE HTTP RESPONSE
        //
        // `http::read()` handles:
        //
        //     TCP bytes
        //          ↓
        //     HTTP parser
        //          ↓
        //     response object
        //
        // It keeps reading until the complete HTTP message has been received.
        // ---------------------------------------------------------------------

        http::read(stream, buffer, response);


        // ---------------------------------------------------------------------
        // 11. DISPLAY THE RESPONSE
        // ---------------------------------------------------------------------

        std::cout << "\n===== RESPONSE =====\n";

        std::cout << "HTTP version : "<< response.version()<< '\n';

        std::cout << "Status       : "<< response.result_int()<< " "<< response.reason()<< '\n';

        std::cout << "Content-Type : "<< response[http::field::content_type]<< '\n';

        std::cout << "Body size    : "<< response.body().size()<< " bytes\n";


        // ---------------------------------------------------------------------
        // 12. PRINT BODY
        // ---------------------------------------------------------------------

        std::cout << "\n===== BODY =====\n";
        std::cout << response.body() << '\n';


        // ---------------------------------------------------------------------
        // 13. SHUT DOWN THE TCP CONNECTION
        //
        // HTTP/1.1 normally supports persistent connections.
        //
        // We explicitly close this connection because this small example
        // has no additional requests to send.
        //
        // `shutdown()` closes the underlying TCP connection.
        // ---------------------------------------------------------------------

        beast::error_code ec;

        stream.socket().shutdown(tcp::socket::shutdown_both,ec);

        // EOF / not-connected errors during shutdown are generally harmless.
        // For a real application, error handling should be more deliberate.
        if (ec){
            std::cerr << "Shutdown error: "<< ec.message()<< '\n';
        }
    }
    catch (const std::exception& e)
    {
        // Synchronous Beast/Asio operations may throw exceptions.
        std::cerr << "Error: "<< e.what()<< '\n';

        return 1;
    }

    return 0;
}