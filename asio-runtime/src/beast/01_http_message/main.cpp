#include <boost/beast/http.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http  = beast::http;

int main()
{
    // -------------------------------------------------------------------------
    // 1. CREATE AN HTTP REQUEST
    //
    // An HTTP request consists of:
    //   - HTTP method      -> GET, POST, PUT, DELETE, ...
    //   - target          -> /api/users
    //   - HTTP version     -> 1.0 / 1.1
    //   - headers          -> Host, Content-Type, Authorization, ...
    //   - body             -> optional payload
    //
    // `string_body` means Beast stores the HTTP body in a std::string.
    // -------------------------------------------------------------------------

    http::request<http::string_body> request;

    // HTTP method.
    request.method(http::verb::get);

    // Resource requested by the client.
    request.target("/api/users");

    // HTTP/1.1 is represented by numeric version 11.
    request.version(11);

    // -------------------------------------------------------------------------
    // 2. ADD HTTP HEADERS
    //
    // `set()` replaces an existing field with the supplied value.
    // Beast provides strongly-typed names for standard HTTP fields.
    // -------------------------------------------------------------------------

    request.set(http::field::host, "example.com");

    request.set(http::field::user_agent, "BeastClient/1.0");

    request.set(http::field::accept, "application/json");

    // -------------------------------------------------------------------------
    // 3. PRINT THE REQUEST
    //
    // Beast's operator<< serializes the HTTP message into HTTP wire format.
    // This is essentially what would eventually be sent over TCP.
    // -------------------------------------------------------------------------

    std::cout << "===== HTTP REQUEST =====\n";
    std::cout << request << '\n';


    // -------------------------------------------------------------------------
    // 4. CREATE AN HTTP POST REQUEST
    //
    // The body type is still `string_body`.
    // This allows us to store arbitrary text such as JSON.
    // -------------------------------------------------------------------------

    http::request<http::string_body> post_request;

    post_request.method(http::verb::post);

    post_request.target("/api/users");

    post_request.version(11);

    post_request.set(http::field::host, "example.com");

    post_request.set(http::field::content_type, "application/json");

    // The actual HTTP payload.
    post_request.body() = R"({"name":"Aftab","role":"student??"})";

    // -------------------------------------------------------------------------
    // `prepare_payload()` calculates headers related to the body.
    //
    // For this string body Beast will, among other things, determine the
    // Content-Length required to transmit the payload correctly.
    // -------------------------------------------------------------------------

    post_request.prepare_payload();

    std::cout << "===== HTTP POST REQUEST =====\n";
    std::cout << post_request << '\n';


    // -------------------------------------------------------------------------
    // 5. READ REQUEST INFORMATION
    // -------------------------------------------------------------------------

    std::cout << "===== REQUEST INFORMATION =====\n";

    std::cout << "Method : "<< request.method_string()<< '\n';

    std::cout << "Target : "<< request.target()<< '\n';

    std::cout << "Version: "<< request.version()<< '\n';

    // Access a header using Beast's field enum.
    std::cout << "Host   : "<< request[http::field::host]<< '\n';


    // -------------------------------------------------------------------------
    // 6. CREATE AN HTTP RESPONSE
    //
    // A response consists of:
    //   - status code
    //   - HTTP version
    //   - headers
    //   - optional body
    // -------------------------------------------------------------------------

    http::response<http::string_body> response;

    // HTTP/1.1.
    response.version(11);

    // HTTP 200 OK.
    response.result(http::status::ok);

    // Tell the client what type of content is being returned.
    response.set(http::field::content_type, "application/json");

    // Response payload.
    response.body() = R"({"status":"success","message":"request processed"})";

    // Calculate Content-Length and other body-related fields.
    response.prepare_payload();

    std::cout << "\n===== HTTP RESPONSE =====\n";
    std::cout << response << '\n';


    // -------------------------------------------------------------------------
    // 7. READ RESPONSE INFORMATION
    // -------------------------------------------------------------------------

    std::cout << "===== RESPONSE INFORMATION =====\n";

    std::cout << "Status : "<< response.result_int()<< '\n';

    std::cout << "Reason : "<< response.reason()<< '\n';

    std::cout << "Content-Type: "<< response[http::field::content_type]<< '\n';

    std::cout << "Body   : "<< response.body()<< '\n';


    // -------------------------------------------------------------------------
    // IMPORTANT CONCEPT
    //
    // Beast's HTTP message is just the protocol representation.
    //
    // At this point:
    //
    //     request/response
    //             │
    //             ▼
    //       C++ data structure
    //
    // There is NO network connection yet.
    //
    // Later, Beast will serialize these objects to/from network buffers:
    //
    //     C++ request
    //          │
    //          ▼
    //     HTTP serializer
    //          │
    //          ▼
    //       TCP bytes
    //
    // and on the receiving side:
    //
    //       TCP bytes
    //          │
    //          ▼
    //      HTTP parser
    //          │
    //          ▼
    //     C++ request
    // -------------------------------------------------------------------------

    return 0;
}