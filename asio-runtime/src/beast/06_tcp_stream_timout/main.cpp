#include <boost/beast/core.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace net   = boost::asio;

using tcp = net::ip::tcp;

int main()
{
    try
    {
        // =========================================================================
        // 1. CREATE THE ASIO EXECUTION CONTEXT
        //
        // io_context owns the asynchronous I/O machinery used by Asio/Beast.
        //
        // This example uses synchronous networking, but tcp_stream's timeout
        // mechanism is still asynchronous internally.
        // =========================================================================

        net::io_context ioc;


        // =========================================================================
        // 2. RESOLVE THE SERVER
        //
        // DNS:
        //
        //     example.com
        //          ↓
        //     IP address + port
        // =========================================================================

        const std::string host = "example.com";
        const std::string port = "80";

        tcp::resolver resolver{ioc};

        auto endpoints = resolver.resolve(host, port);


        // =========================================================================
        // 3. CREATE A BEAST TCP STREAM
        //
        // tcp_stream wraps an Asio TCP socket and provides Beast-specific
        // functionality, most importantly convenient timeout handling.
        //
        // Conceptually:
        //
        //     tcp_stream
        //          │
        //          └── tcp::socket
        // =========================================================================

        beast::tcp_stream stream{ioc};


        // =========================================================================
        // 4. SET A CONNECT TIMEOUT
        //
        // If establishing the TCP connection takes longer than this duration,
        // Beast will cancel the operation.
        //
        // This prevents a connection attempt from hanging indefinitely.
        // =========================================================================

        stream.expires_after(std::chrono::seconds(5));


        // =========================================================================
        // 5. CONNECT TO THE SERVER
        //
        // tcp_stream::connect() tries the resolved endpoints.
        //
        // The timeout configured above applies to the operation.
        // =========================================================================

        stream.connect(endpoints);

        std::cout << "Connected to "
                  << host
                  << '\n';


        // =========================================================================
        // 6. CHANGE THE TIMEOUT FOR THE NEXT OPERATION
        //
        // Timeouts are operation-oriented.
        //
        // For example:
        //
        //     connect timeout -> 5 seconds
        //     read timeout    -> 10 seconds
        //
        // We can change the expiration before another operation.
        // =========================================================================

        stream.expires_after(std::chrono::seconds(10));


        // =========================================================================
        // 7. PERFORM A RAW TCP WRITE
        //
        // This example deliberately does NOT use Beast HTTP.
        //
        // We are demonstrating tcp_stream itself.
        //
        // The bytes below form a minimal HTTP request manually.
        // =========================================================================

        const std::string request =
            "GET / HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n"
            "\r\n";


        net::write(stream, net::buffer(request));

        std::cout << "HTTP request sent\n";


        // =========================================================================
        // 8. READ RAW TCP DATA
        //
        // tcp_stream is still fundamentally a byte stream.
        //
        // It does not parse HTTP.
        //
        // Beast HTTP parsing is a separate layer.
        // =========================================================================

        char data[4096];

        beast::error_code ec;

        std::size_t bytes_read = stream.read_some(
            net::buffer(data),
            ec
        );


        // =========================================================================
        // 9. HANDLE THE READ RESULT
        //
        // Because the server requested:
        //
        //     Connection: close
        //
        // the remote side may close the connection after sending the response.
        //
        // EOF is therefore expected in many HTTP/1.0-style/raw examples.
        // =========================================================================

        if (ec && ec != net::error::eof)
        {
            throw beast::system_error(ec);
        }


        std::cout << "\n===== RECEIVED DATA =====\n";

        std::cout.write(data, bytes_read);

        std::cout << "\n";


        // =========================================================================
        // 10. RESET / DISABLE THE EXPIRATION
        //
        // `expires_never()` removes the current operation timeout.
        //
        // This is useful when the stream will remain alive for a long time,
        // such as a persistent connection or WebSocket.
        // =========================================================================

        stream.expires_never();


        // =========================================================================
        // 11. SHUT DOWN THE CONNECTION
        // =========================================================================

        stream.socket().shutdown(
            tcp::socket::shutdown_both,
            ec
        );

        // During shutdown, the peer may already have closed the connection.
        // That is normally harmless.
        if (ec && ec != net::error::not_connected)
        {
            std::cerr << "Shutdown error: "
                      << ec.message()
                      << '\n';
        }


        // =========================================================================
        // IMPORTANT CONCEPT
        //
        // tcp_stream does NOT replace Beast HTTP.
        //
        // Think of the layers like this:
        //
        //     HTTP
        //       │
        //       ▼
        //     tcp_stream
        //       │
        //       ▼
        //     TCP socket
        //
        // tcp_stream handles the transport-level connection and gives us
        // convenient timeout control.
        //
        // HTTP parsing/serialization remains the responsibility of Beast HTTP.
        // =========================================================================
    }
    catch (const beast::system_error& e)
    {
        // Beast/Asio errors contain both:
        //
        //     error code
        //     error message
        //
        // system_error preserves that information while also behaving like
        // a standard C++ exception.
        std::cerr << "Beast error: "
                  << e.code().message()
                  << '\n';

        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: "
                  << e.what()
                  << '\n';

        return 1;
    }

    return 0;
}