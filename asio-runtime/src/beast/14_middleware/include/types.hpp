#pragma once

#include <boost/beast/http.hpp>

#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace beast = boost::beast;
namespace http  = beast::http;

// =============================================================================
// HTTP TYPES
// =============================================================================

using Request =
    http::request<http::string_body>;

using Response =
    http::response<http::string_body>;


// =============================================================================
// REQUEST CONTEXT
// =============================================================================

struct RequestContext
{
    std::string request_id;

    std::chrono::steady_clock::time_point start_time;
};


// =============================================================================
// ROUTE PARAMETERS
// =============================================================================

using Parameters =
    std::unordered_map<
        std::string,
        std::string
    >;


// =============================================================================
// QUERY PARAMETERS
// =============================================================================

using QueryParameters =
    std::unordered_map<
        std::string,
        std::string
    >;


// =============================================================================
// ROUTE HANDLER
// =============================================================================

using Handler =
    std::function<
        Response(
            const Request&,
            RequestContext&,
            const Parameters&,
            const QueryParameters&
        )
    >;


// =============================================================================
// MIDDLEWARE
// =============================================================================

using Next =
    std::function<Response()>;


using Middleware =
    std::function<
        Response(
            const Request&,
            RequestContext&,
            Next
        )
    >;