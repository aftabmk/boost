#pragma once

#include <boost/beast/http.hpp>

#include <functional>

namespace http = boost::beast::http;


// =============================================================================
// HTTP TYPES
// =============================================================================

using Request =
    http::request<http::string_body>;


using Response =
    http::response<http::string_body>;


// =============================================================================
// ROUTE HANDLER
// =============================================================================
//
// Request
//    ↓
// Handler
//    ↓
// Response
//
// =============================================================================

using Handler =
    std::function<Response(const Request&)>;