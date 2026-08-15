#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <chrono>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// PRINT ERROR INFORMATION
// =============================================================================

void print_error(const std::string& operation,const beast::error_code& ec) {
    // -------------------------------------------------------------------------
    // error_code contains:
    //
    //     category
    //     numeric error value
    //     human-readable message
    //
    // Never compare network errors only by their text.
    // Use the actual error_code / error category.
    // -------------------------------------------------------------------------

    std::cout<< operation<< "\n  value   : " << ec.value()<< "\n  message : " << ec.message()
        << "\n  category: " << ec.category().name() << "\n\n";
}


// =============================================================================
// DEMONSTRATE COMMON BEAST ERROR CONDITIONS
// =============================================================================

void demonstrate_error_codes()
{
    // =========================================================================
    // 1. CREATE ERROR CODES
    //
    // error_code starts in the "no error" state.
    // =========================================================================

    beast::error_code ec;

    std::cout << "Initial error state:\n";
    std::cout << "value   : " << ec.value() << '\n';
    std::cout << "message : " << ec.message() << '\n';
    std::cout<< "failed  : " << std::boolalpha << static_cast<bool>(ec)<< "\n\n";


    // =========================================================================
    // 2. BEAST HTTP `NEED_MORE`
    //
    // `need_more` is a protocol/parser state, not necessarily a fatal error.
    //
    // It means:
    //
    //     "The data received so far is valid, but the parser needs more data."
    //
    // This is very different from malformed HTTP.
    // =========================================================================

    ec = http::error::need_more;

    print_error("HTTP parser needs more data",ec);


    // =========================================================================
    // 3. EOF
    //
    // EOF means the peer closed its side of the stream.
    //
    // This is often NORMAL in networking.
    //
    // For example:
    //
    //     client sends request
    //     server sends response
    //     server closes connection
    //
    // A read operation can then report EOF.
    // =========================================================================

    ec = net::error::eof;

    print_error("Peer closed connection",ec);


    // =========================================================================
    // 4. OPERATION ABORTED
    //
    // An asynchronous operation can be cancelled.
    //
    // For example:
    //
    //     timeout occurs
    //          ↓
    //     socket operation cancelled
    //          ↓
    //     operation_aborted
    //
    // This does not necessarily mean the server itself failed.
    // =========================================================================

    ec = net::error::operation_aborted;

    print_error("Operation cancelled",ec);
}


// =============================================================================
// HTTP SERVER WITH ERROR HANDLING
// =============================================================================

void run_server() {
    try {
        // ---------------------------------------------------------------------
        // 1. IO CONTEXT
        // ---------------------------------------------------------------------

        net::io_context ioc;


        // ---------------------------------------------------------------------
        // 2. LISTENING SOCKET
        // ---------------------------------------------------------------------

        tcp::acceptor acceptor(
            ioc,tcp::endpoint(tcp::v4(), 8080)
        );

        std::cout << "Server listening on http://127.0.0.1:8080\n";


        // ---------------------------------------------------------------------
        // 3. ACCEPT CONNECTIONS
        //
        // This server handles one connection at a time because this is still
        // a synchronous educational example.
        // ---------------------------------------------------------------------

        for (;;) {
            tcp::socket socket(ioc);

            beast::error_code ec;


            // =================================================================
            // ACCEPT
            // =================================================================

            acceptor.accept(socket, ec);

            if (ec) {
                // -----------------------------------------------------------------
                // An accept failure is different from a client disconnect.
                //
                // The acceptor itself failed, so we report the error and
                // terminate this example.
                // -----------------------------------------------------------------

                print_error("accept() failed",ec);

                break;
            }


            std::cout << "\nClient connected\n";


            // =================================================================
            // CONFIGURE A TIMEOUT
            //
            // tcp_stream is useful here because Beast can manage the timeout
            // around the socket operation.
            // =================================================================

            beast::tcp_stream stream(std::move(socket));

            stream.expires_after(std::chrono::seconds(10));


            // =================================================================
            // HTTP REQUEST
            // =================================================================

            beast::flat_buffer buffer;

            http::request<http::string_body> request;


            // =================================================================
            // READ REQUEST
            // =================================================================

            http::read(stream,buffer,request,ec);


            // =================================================================
            // HANDLE READ ERROR
            // =================================================================

            if (ec) {
                // -------------------------------------------------------------
                // CASE 1: CLIENT CLOSED THE CONNECTION
                //
                // This is usually not a server failure.
                // -------------------------------------------------------------

                if (ec == net::error::eof) {
                    std::cout
                        << "Client closed connection before sending "
                           "a complete HTTP request\n";

                    continue;
                }


                // -------------------------------------------------------------
                // CASE 2: OPERATION TIMED OUT
                //
                // A slow/malfunctioning client may never finish its request.
                //
                // In a real server you would normally close this connection.
                // -------------------------------------------------------------

                if (ec == beast::error::timeout){
                    std::cout << "Client request timed out\n";

                    continue;
                }


                // -------------------------------------------------------------
                // CASE 3: CONNECTION RESET
                //
                // The peer disappeared unexpectedly.
                //
                // Again, this is generally a connection-level event rather
                // than an application-wide server failure.
                // -------------------------------------------------------------

                if (ec == net::error::connection_reset) {
                    std::cout << "Client reset the connection\n";

                    continue;
                }


                // -------------------------------------------------------------
                // CASE 4: SOMETHING ELSE
                //
                // This could be malformed HTTP, a resource problem, etc.
                // Report it.
                // -------------------------------------------------------------

                print_error("HTTP read failed",ec);

                continue;
            }


            // =================================================================
            // REQUEST WAS SUCCESSFULLY PARSED
            // =================================================================

            std::cout << "Request received: " << request.method_string()
                << " "<< request.target()<< '\n';


            // =================================================================
            // CREATE RESPONSE
            // =================================================================

            http::response<http::string_body> response(http::status::ok,request.version());

            response.set(http::field::content_type,"text/plain");

            response.body() = "Hello from error-aware Beast server!\n";

            response.keep_alive(request.keep_alive());

            response.prepare_payload();


            // =================================================================
            // WRITE RESPONSE
            // =================================================================

            http::write(stream,response,ec);


            // =================================================================
            // HANDLE WRITE ERROR
            // =================================================================

            if (ec) {
                // -------------------------------------------------------------
                // A client can disappear between the request and response.
                //
                // For example:
                //
                //     server reads request
                //           ↓
                //     client closes browser
                //           ↓
                //     server tries write()
                //           ↓
                //     connection reset / broken pipe
                //
                // That should usually only terminate THIS connection.
                // -------------------------------------------------------------

                if (ec == net::error::connection_reset) {
                    std::cout << "Client disconnected while sending response\n";
                }
                else {
                    print_error("HTTP write failed",ec);
                }

                continue;
            }


            std::cout << "Response sent successfully\n";


            // =================================================================
            // CONNECTION LIFETIME
            // =================================================================

            if (!response.keep_alive()) {
                beast::error_code shutdown_ec;

                stream.socket().shutdown(
                    tcp::socket::shutdown_send,
                    shutdown_ec
                );


                // -------------------------------------------------------------
                // Shutdown errors are often harmless if the peer already
                // closed the connection.
                // -------------------------------------------------------------

                if (shutdown_ec && shutdown_ec != net::error::not_connected){
                    print_error("socket shutdown",shutdown_ec);
                }
            }
        }
    }
    catch (const beast::system_error& e) {
        // ---------------------------------------------------------------------
        // system_error is useful when you intentionally want exception-based
        // handling instead of passing error_code objects everywhere.
        //
        // `code()` still gives you the structured error information.
        // ---------------------------------------------------------------------

        std::cerr
            << "Beast system error:\n"
            << "  value   : " << e.code().value() << '\n'
            << "  message : " << e.code().message() << '\n'
            << "  category: " << e.code().category().name() << '\n';
    }
    catch (const std::exception& e) {
        // ---------------------------------------------------------------------
        // Unexpected non-network exception.
        // ---------------------------------------------------------------------

        std::cerr<< "Unexpected exception: "<< e.what()<< '\n';
    }
}


// =============================================================================
// MAIN
// =============================================================================

int main() {
    // =========================================================================
    // First demonstrate the error_code model.
    // =========================================================================

    demonstrate_error_codes();


    // =========================================================================
    // Then start the actual HTTP server.
    // =========================================================================

    run_server();

    return 0;
}