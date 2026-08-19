#pragma once

#include "types.hpp"
#include "url.hpp"

#include <boost/beast/http.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace http = boost::beast::http;


// =============================================================================
// ROUTE
// =============================================================================

struct Route
{
    http::verb method;

    std::vector<std::string> segments;

    Handler handler;
};


// =============================================================================
// ROUTER
// =============================================================================

class Router
{
private:

    std::vector<Route> routes;


public:

    // =========================================================================
    // ADD
    // =========================================================================

    void add(
        http::verb method,
        std::string_view pattern,
        Handler handler)
    {
        routes.push_back(
            Route{
                method,
                split_path(pattern),
                std::move(handler)
            }
        );
    }


    // =========================================================================
    // HANDLE
    // =========================================================================

    Response handle(
        const Request& request,
        RequestContext& context)
    {
        Url url =
            Url::parse(
                request.target()
            );


        for (const Route& route : routes)
        {
            if (route.method != request.method())
                continue;


            Parameters parameters;


            if (!match_path(
                    route.segments,
                    url.path,
                    parameters))
            {
                continue;
            }


            return route.handler(
                request,
                context,
                parameters,
                url.query
            );
        }


        // =====================================================================
        // 404
        // =====================================================================

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


private:

    // =========================================================================
    // SPLIT PATH
    // =========================================================================

    static std::vector<std::string> split_path(
        std::string_view path)
    {
        std::vector<std::string> result;


        if (!path.empty() &&
            path.front() == '/')
        {
            path.remove_prefix(1);
        }


        if (path.empty())
            return result;


        while (!path.empty())
        {
            const std::size_t slash =
                path.find('/');


            if (slash == std::string_view::npos)
            {
                result.emplace_back(path);
                break;
            }


            result.emplace_back(
                path.substr(
                    0,
                    slash
                )
            );


            path =
                path.substr(
                    slash + 1
                );
        }


        return result;
    }


    // =========================================================================
    // MATCH PATH
    // =========================================================================

    static bool match_path(
        const std::vector<std::string>& pattern,
        std::string_view path,
        Parameters& parameters)
    {
        std::vector<std::string> actual =
            split_path(path);


        if (pattern.size() != actual.size())
            return false;


        for (std::size_t i = 0;
             i < pattern.size();
             ++i)
        {
            const std::string& expected =
                pattern[i];

            const std::string& received =
                actual[i];


            // -----------------------------------------------------------------
            // Dynamic path parameter.
            //
            // /users/:id
            // -----------------------------------------------------------------

            if (!expected.empty() &&
                expected.front() == ':')
            {
                parameters.emplace(
                    expected.substr(1),
                    received
                );

                continue;
            }


            // -----------------------------------------------------------------
            // Static path segment.
            // -----------------------------------------------------------------

            if (expected != received)
                return false;
        }


        return true;
    }
};