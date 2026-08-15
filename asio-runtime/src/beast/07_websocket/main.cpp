#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// WEBSOCKET SERVER
// =============================================================================

void run_server()
{
    try
    {
        // ---------------------------------------------------------------------
        // 1. CREATE SERVER I/O CONTEXT
        // ---------------------------------------------------------------------

        net::io_context ioc;


        // ---------------------------------------------------------------------
        // 2. CREATE TCP ACCEPTOR
        //
        // The acceptor listens for incoming TCP connections.
        // WebSocket itself comes AFTER the TCP connection is established.
        // ---------------------------------------------------------------------

        tcp::acceptor acceptor(
            ioc,
            tcp::endpoint(tcp::v4(), 8080)
        );

        std::cout << "[Server] Listening on port 8080...\n";


        // ---------------------------------------------------------------------
        // 3. ACCEPT ONE TCP CONNECTION
        // ---------------------------------------------------------------------

        tcp::socket socket(ioc);

        acceptor.accept(socket);

        std::cout << "[Server] TCP connection accepted\n";


        // ---------------------------------------------------------------------
        // 4. CREATE WEBSOCKET STREAM
        //
        // This wraps the TCP socket with WebSocket protocol functionality.
        //
        // Conceptually:
        //
        //     websocket::stream
        //           ↓
        //       tcp::socket
        //           ↓
        //           TCP
        // ---------------------------------------------------------------------

        websocket::stream<tcp::socket> ws(std::move(socket));


        // ---------------------------------------------------------------------
        // 5. ACCEPT THE WEBSOCKET HANDSHAKE
        //
        // The client initially connects using HTTP.
        //
        // It then requests:
        //
        //     HTTP Upgrade: websocket
        //
        // Beast handles the WebSocket handshake for us.
        // ---------------------------------------------------------------------

        ws.accept();

        std::cout << "[Server] WebSocket handshake completed\n";


        // ---------------------------------------------------------------------
        // 6. CREATE BUFFER FOR INCOMING MESSAGE
        //
        // WebSocket messages can arrive in multiple frames.
        //
        // Beast manages the framing and reconstructs the message into this
        // buffer.
        // ---------------------------------------------------------------------

        beast::flat_buffer buffer;


        // ---------------------------------------------------------------------
        // 7. READ A WEBSOCKET MESSAGE
        //
        // Unlike raw TCP, WebSocket is message-oriented.
        //
        // We don't manually parse:
        //
        //     frame length
        //     opcode
        //     masking
        //     continuation frames
        //
        // Beast handles those protocol details.
        // ---------------------------------------------------------------------

        ws.read(buffer);


        // ---------------------------------------------------------------------
        // 8. CHECK WHETHER THE MESSAGE IS TEXT OR BINARY
        //
        // WebSocket messages have an opcode indicating their type.
        //
        // `got_text()` tells us whether the received message is a text message.
        // ---------------------------------------------------------------------

        std::cout << "[Server] Received "<< (ws.got_text() ? "TEXT" : "BINARY")<< " message\n";


        // ---------------------------------------------------------------------
        // 9. EXTRACT MESSAGE CONTENT
        // ---------------------------------------------------------------------

        std::string message(
            static_cast<const char*>(buffer.data().data()),
            buffer.size()
        );

        std::cout << "[Server] Message: "<< message<< '\n';


        // ---------------------------------------------------------------------
        // 10. SEND A RESPONSE
        //
        // `text(true)` tells Beast that the next outgoing message is a text
        // WebSocket message.
        //
        // `write()` then creates the appropriate WebSocket frames.
        // ---------------------------------------------------------------------

        ws.text(true);

        const std::string response = "Echo from server: " + message;

        ws.write(net::buffer(response));

        std::cout << "[Server] Response sent\n";


        // ---------------------------------------------------------------------
        // 11. CLOSE THE WEBSOCKET
        //
        // WebSocket has its own close handshake.
        //
        // This is different from simply closing the underlying TCP socket.
        // ---------------------------------------------------------------------

        ws.close(websocket::close_code::normal);

        std::cout << "[Server] WebSocket closed\n";
    }
    catch (const beast::system_error& e) {
        std::cerr << "[Server] Beast error: "<< e.code().message()<< '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "[Server] Error: "<< e.what()<< '\n';
    }
}


// =============================================================================
// WEBSOCKET CLIENT
// =============================================================================

void run_client() {
    try
    {
        // ---------------------------------------------------------------------
        // Give the server thread a moment to start listening.
        //
        // This is only for this small self-contained demonstration.
        // Production code should synchronize startup properly.
        // ---------------------------------------------------------------------

        std::this_thread::sleep_for(std::chrono::milliseconds(100));


        // ---------------------------------------------------------------------
        // 1. CREATE CLIENT I/O CONTEXT
        // ---------------------------------------------------------------------

        net::io_context ioc;


        // ---------------------------------------------------------------------
        // 2. RESOLVE SERVER
        // ---------------------------------------------------------------------

        tcp::resolver resolver(ioc);

        auto endpoints = resolver.resolve("127.0.0.1","8080");


        // ---------------------------------------------------------------------
        // 3. CREATE TCP SOCKET
        // ---------------------------------------------------------------------

        tcp::socket socket(ioc);


        // ---------------------------------------------------------------------
        // 4. CONNECT TCP
        // ---------------------------------------------------------------------

        net::connect(socket, endpoints);

        std::cout << "[Client] TCP connection established\n";


        // ---------------------------------------------------------------------
        // 5. WRAP TCP SOCKET WITH WEBSOCKET
        // ---------------------------------------------------------------------

        websocket::stream<tcp::socket> ws(std::move(socket));


        // ---------------------------------------------------------------------
        // 6. PERFORM WEBSOCKET CLIENT HANDSHAKE
        //
        // The initial protocol transition is:
        //
        //     TCP
        //      ↓
        //     HTTP Upgrade
        //      ↓
        //     WebSocket
        //
        // Beast handles the handshake protocol.
        // ---------------------------------------------------------------------

        ws.handshake( "localhost", "/");

        std::cout << "[Client] WebSocket handshake completed\n";


        // ---------------------------------------------------------------------
        // 7. SEND TEXT MESSAGE
        //
        // WebSocket is message-oriented.
        //
        // We tell Beast that the outgoing message should be a text message.
        // ---------------------------------------------------------------------

        ws.text(true);

        const std::string message = "Hello WebSocket!";

        ws.write(net::buffer(message));

        std::cout << "[Client] Message sent\n";


        // ---------------------------------------------------------------------
        // 8. CREATE RECEIVE BUFFER
        // ---------------------------------------------------------------------

        beast::flat_buffer buffer;


        // ---------------------------------------------------------------------
        // 9. RECEIVE SERVER RESPONSE
        // ---------------------------------------------------------------------

        ws.read(buffer);


        // ---------------------------------------------------------------------
        // 10. DISPLAY RESPONSE
        // ---------------------------------------------------------------------

        std::string response(
            static_cast<const char*>(buffer.data().data()),
            buffer.size()
        );

        std::cout << "[Client] Received: "<< response<< '\n';


        // ---------------------------------------------------------------------
        // 11. CLOSE WEBSOCKET
        // ---------------------------------------------------------------------

        ws.close(websocket::close_code::normal);

        std::cout << "[Client] WebSocket closed\n";
    }
    catch (const beast::system_error& e) {
        std::cerr << "[Client] Beast error: "<< e.code().message()<< '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "[Client] Error: "<< e.what()<< '\n';
    }
}


// =============================================================================
// MAIN
// =============================================================================

int main() {
    // =========================================================================
    // Start the server in a separate thread.
    //
    // This lets the same executable contain both sides of the demonstration.
    // =========================================================================

    std::thread server_thread(run_server);


    // =========================================================================
    // Run the client on the main thread.
    // =========================================================================

    run_client();


    // =========================================================================
    // Wait until the server finishes.
    // =========================================================================

    server_thread.join();

    return 0;
}