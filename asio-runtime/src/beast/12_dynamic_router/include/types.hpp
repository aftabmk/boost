#pragma once

#include <boost/beast/http.hpp>

#include <functional>
#include <string>
#include <unordered_map>

namespace beast_server
{

namespace http = boost::beast::http;

using Request =
    http::request<http::string_body>;

using Response =
    http::response<http::string_body>;

using Parameters =
    std::unordered_map<
        std::string,
        std::string
    >;

using Handler =
    std::function<
        Response(
            const Request&,
            const Parameters&,
            const Parameters&
        )
    >;

}