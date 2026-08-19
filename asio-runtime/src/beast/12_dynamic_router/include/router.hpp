#pragma once

#include "types.hpp"
#include "url.hpp"

#include <boost/beast/http.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace beast_server
{

namespace http = boost::beast::http;


// =============================================================================
// ROUTE MATCH
// =============================================================================

struct RouteMatch
{
    bool matched = false;

    Parameters params;
};


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
        Route route;

        route.method =
            method;

        route.segments =
            split_path(pattern);

        route.handler =
            std::move(handler);

        routes.emplace_back(
            std::move(route)
        );
    }


    // =========================================================================
    // HANDLE
    // =========================================================================

    Response handle(
        const Request& request) const
    {
        Url url =
            Url::parse(
                request.target()
            );


        for (const Route& route : routes)
        {
            if (route.method != request.method())
                continue;


            RouteMatch match =
                match_path(
                    route.segments,
                    url.path
                );


            if (!match.matched)
                continue;


            return route.handler(
                request,
                match.params,
                url.query
            );
        }


        return not_found(
            request
        );
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

    static RouteMatch match_path(
        const std::vector<std::string>& pattern,
        std::string_view path)
    {
        RouteMatch result;


        std::vector<std::string> actual =
            split_path(path);


        if (pattern.size() != actual.size())
            return result;


        for (std::size_t i = 0;
             i < pattern.size();
             ++i)
        {
            const std::string& expected =
                pattern[i];

            const std::string& received =
                actual[i];


            // -------------------------------------------------------------
            // Dynamic parameter
            //
            // :id
            // :user
            // :name
            // -------------------------------------------------------------

            if (!expected.empty() &&
                expected.front() == ':')
            {
                result.params.emplace(
                    expected.substr(1),
                    received
                );

                continue;
            }


            // -------------------------------------------------------------
            // Static segment
            // -------------------------------------------------------------

            if (expected != received)
                return result;
        }


        result.matched = true;

        return result;
    }


    // =========================================================================
    // 404
    // =========================================================================

    static Response not_found(
        const Request& request)
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
};

}