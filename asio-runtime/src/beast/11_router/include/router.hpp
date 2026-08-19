#pragma once

#include "types.hpp"

#include <boost/beast/http.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace http = boost::beast::http;


// =============================================================================
// ROUTER
// =============================================================================
//
// HTTP request
//      │
//      ├── method
//      └── path
//           │
//           ▼
//        RouteKey
//           │
//           ▼
//        Handler
//           │
//           ▼
//       Response
//
// Router knows nothing about:
//
//     TCP
//     sockets
//     io_context
//     threads
//     async operations
//
// =============================================================================

class Router
{
private:

    // =========================================================================
    // ROUTE KEY
    // =========================================================================

    struct RouteKey
    {
        http::verb method;

        std::string path;


        bool operator==(
            const RouteKey& other) const
        {
            return method == other.method &&
                   path == other.path;
        }
    };


    // =========================================================================
    // ROUTE KEY HASH
    // =========================================================================

    struct RouteKeyHash
    {
        std::size_t operator()(
            const RouteKey& key) const
        {
            const std::size_t h1 =
                std::hash<int>{}(
                    static_cast<int>(key.method)
                );


            const std::size_t h2 =
                std::hash<std::string>{}(
                    key.path
                );


            return h1 ^
                   (h2 + 0x9e3779b9 +
                    (h1 << 6) +
                    (h1 >> 2));
        }
    };


    // =========================================================================
    // ROUTE TABLE
    // =========================================================================

    std::unordered_map<
        RouteKey,
        Handler,
        RouteKeyHash
    > routes;


public:

    // =========================================================================
    // ADD
    // =========================================================================

    void add(
        http::verb method,
        std::string_view path,
        Handler handler)
    {
        routes.emplace(
            RouteKey{
                method,
                std::string(path)
            },
            std::move(handler)
        );
    }


    // =========================================================================
    // HANDLE
    // =========================================================================

    Response handle(
        const Request& request) const
    {
        std::string path =
            std::string(request.target());


        auto it =
            routes.find(
                RouteKey{
                    request.method(),
                    std::move(path)
                }
            );


        if (it == routes.end())
        {
            Response response{
                http::status::not_found,
                request.version()
            };


            response.set(
                http::field::content_type,
                "text/plain"
            );


            response.body() =
                "404 - Route not found\n";


            response.keep_alive(
                request.keep_alive()
            );


            response.prepare_payload();

            return response;
        }


        return it->second(request);
    }
};