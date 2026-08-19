#pragma once

#include <boost/beast/http.hpp>

namespace beast = boost::beast;
namespace http  = beast::http;

using Request =
    http::request<http::string_body>;

using Response =
    http::response<http::string_body>;