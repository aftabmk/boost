#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/buffer.hpp>

#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;

int main()
{
    // -------------------------------------------------------------------------
    // Example HTTP message.
    //
    // In a real server these bytes would arrive from a TCP socket.
    //
    // We deliberately split the message into multiple chunks below to
    // demonstrate incremental parsing.
    // -------------------------------------------------------------------------

    const std::string part1 = "GET /api/users HTTP/1.1\r\n" "Host: example.com\r\n" "User-Agent: BeastClient\r\n";

    const std::string part2 = "Accept: application/json\r\n" "\r\n";


    // -------------------------------------------------------------------------
    // 1. CREATE THE PARSER
    //
    // `request_parser<string_body>` parses an HTTP request whose body will
    // eventually be stored in std::string.
    //
    // Unlike `http::read()`, we control when bytes are supplied to the parser.
    // -------------------------------------------------------------------------

    http::request_parser<http::string_body> parser;


    // -------------------------------------------------------------------------
    // 2. FIRST CHUNK
    //
    // Imagine TCP gave us only this data:
    //
    //     GET /api/users HTTP/1.1
    //     Host: example.com
    //     User-Agent: BeastClient
    //
    // The HTTP message is NOT complete yet.
    // -------------------------------------------------------------------------

    auto result1 = parser.put(net::buffer(part1));


    // -------------------------------------------------------------------------
    // `put()` returns an error_code.
    //
    // `need_more` means:
    //
    //     "The bytes were valid, but I need more data."
    //
    // This is NOT a failure.
    // -------------------------------------------------------------------------

    if (result1) {
        if (result1 == http::error::need_more) {
            std::cout << "Parser needs more data after chunk 1\n";
        }
        else{
            std::cerr<< "Parser error: "<< result1.message()<< '\n';
            return 1;
        }
    }


    // -------------------------------------------------------------------------
    // Check whether the complete HTTP message has arrived.
    // -------------------------------------------------------------------------

    std::cout << "Message complete after chunk 1: " << std::boolalpha << parser.is_done() << '\n';


    // -------------------------------------------------------------------------
    // 3. SECOND CHUNK
    //
    // Now the remaining header and the blank line arrive.
    //
    // The blank line:
    //
    //     \r\n
    //
    // terminates the HTTP headers.
    // -------------------------------------------------------------------------

    auto result2 = parser.put(
        net::buffer(part2)
    );


    if (result2) {
        if (result2 == http::error::need_more) {
            std::cout
                << "Parser needs more data after chunk 2\n";
        }
        else {
            std::cerr << "Parser error: " << result2.message() << '\n';

            return 1;
        }
    }


    // -------------------------------------------------------------------------
    // 4. CHECK WHETHER PARSING IS FINISHED
    // -------------------------------------------------------------------------

    std::cout<< "Message complete after chunk 2: "<< parser.is_done()<< '\n';


    // -------------------------------------------------------------------------
    // 5. ACCESS THE PARSED REQUEST
    //
    // `get()` gives access to the HTTP request that the parser has constructed.
    //
    // At this point:
    //
    //     raw bytes
    //         ↓
    //     parser
    //         ↓
    //     request object
    // -------------------------------------------------------------------------

    auto& request = parser.get();


    // -------------------------------------------------------------------------
    // 6. DISPLAY THE RESULT
    // -------------------------------------------------------------------------

    std::cout << "\n===== PARSED REQUEST =====\n";

    std::cout<< "Method : "<< request.method_string()<< '\n';

    std::cout<< "Target : "<< request.target()<< '\n';

    std::cout<< "Version: "<< request.version()<< '\n';


    // -------------------------------------------------------------------------
    // 7. ACCESS HEADERS
    // -------------------------------------------------------------------------

    std::cout<< "Host   : "<< request[http::field::host]<< '\n';

    std::cout<< "Agent  : "<< request[http::field::user_agent]<< '\n';

    std::cout<< "Accept : "<< request[http::field::accept]<< '\n';


    // -------------------------------------------------------------------------
    // 8. IMPORTANT PARSER STATE
    //
    // Beast's parser tracks how much of the HTTP message has been consumed.
    //
    // This is particularly important when reading from a TCP stream because
    // one network read can contain:
    //
    //     complete HTTP request
    //     +
    //     beginning of the next HTTP request
    //
    // A production server therefore cannot blindly throw away every byte
    // after parsing one message.
    // -------------------------------------------------------------------------

    std::cout<< "\nParser finished: "<< parser.is_done()<< '\n';


    // -------------------------------------------------------------------------
    // 9. BODY INFORMATION
    //
    // This example has no body because it is a GET request.
    // -------------------------------------------------------------------------

    std::cout<< "Body size: "<< request.body().size()<< " bytes\n";


    // -------------------------------------------------------------------------
    // IMPORTANT CONCEPT
    //
    // `http::read()` used in the previous tutorial hides this entire process.
    //
    // Conceptually:
    //
    //     http::read()
    //          │
    //          ├── read TCP bytes
    //          ├── feed parser
    //          ├── read more if necessary
    //          ├── feed parser again
    //          └── stop when parser.is_done()
    //
    // By using the parser directly, YOU control this process.
    // -------------------------------------------------------------------------

    return 0;
}