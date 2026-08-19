#pragma once

#include "route.hpp"
#include "url.hpp"

#include <boost/beast/http/status.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>


class Router
{
private:

    std::vector<Route> routes;


public:

    // =========================================================================
    // ADD ROUTE
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
    // HANDLE REQUEST
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
            // -----------------------------------------------------------------
            // HTTP method.
            // -----------------------------------------------------------------

            if (route.method != request.method())
                continue;


            // -----------------------------------------------------------------
            // Path.
            // -----------------------------------------------------------------

            Parameters params;


            if (!match(
                    route.segments,
                    url.path,
                    params))
            {
                continue;
            }


            // -----------------------------------------------------------------
            // Route found.
            // -----------------------------------------------------------------

            return route.handler(
                request,
                params,
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
    // MATCH
    // =========================================================================

    static bool match(
        const std::vector<std::string>& pattern,
        std::string_view path,
        Parameters& params)
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


            // -------------------------------------------------------------
            // Parameter.
            //
            // /users/:id
            // -------------------------------------------------------------

            if (!expected.empty() &&
                expected.front() == ':')
            {
                params.emplace(
                    expected.substr(1),
                    received
                );

                continue;
            }


            // -------------------------------------------------------------
            // Static segment.
            // -------------------------------------------------------------

            if (expected != received)
                return false;
        }


        return true;
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