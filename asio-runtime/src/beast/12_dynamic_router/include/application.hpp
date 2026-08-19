#pragma once

#include "router.hpp"

#include <boost/beast/http.hpp>

#include <functional>
#include <string>
#include <thread>

namespace beast_server
{

namespace http = boost::beast::http;


class Application
{
private:

    Router router;


public:

    Application()
    {
        register_routes();
    }


    Response handle(
        const Request& request) const
    {
        return router.handle(request);
    }


private:

    // =========================================================================
    // REGISTER ROUTES
    // =========================================================================

    void register_routes()
    {
        // =====================================================================
        // GET /
        // =====================================================================

        router.add(
            http::verb::get,
            "/",

            [](
                const Request& request,
                const auto&,
                const auto&)
            {
                Response response{
                    http::status::ok,
                    request.version()
                };


                response.set(
                    http::field::content_type,
                    "text/plain"
                );


                response.body() =
                    "Dynamic Beast Router\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // =====================================================================
        // GET /users/:id
        // =====================================================================

        router.add(
            http::verb::get,
            "/users/:id",

            [](
                const Request& request,
                const auto& params,
                const auto&)
            {
                Response response{
                    http::status::ok,
                    request.version()
                };


                response.set(
                    http::field::content_type,
                    "text/plain"
                );


                response.body() =
                    "User ID = " +
                    params.at("id") +
                    "\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // =====================================================================
        // GET /search?q=beast
        // =====================================================================

        router.add(
            http::verb::get,
            "/search",

            [](
                const Request& request,
                const auto&,
                const auto& query)
            {
                Response response{
                    http::status::ok,
                    request.version()
                };


                response.set(
                    http::field::content_type,
                    "text/plain"
                );


                auto it =
                    query.find("q");


                if (it == query.end())
                {
                    response.body() =
                        "Missing query parameter: q\n";
                }
                else
                {
                    response.body() =
                        "Search query = " +
                        it->second +
                        "\n";
                }


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // =====================================================================
        // GET /users/:id/profile
        // =====================================================================

        router.add(
            http::verb::get,
            "/users/:id/profile",

            [](
                const Request& request,
                const auto& params,
                const auto&)
            {
                Response response{
                    http::status::ok,
                    request.version()
                };


                response.set(
                    http::field::content_type,
                    "text/plain"
                );


                response.body() =
                    "Profile of user " +
                    params.at("id") +
                    "\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // =====================================================================
        // POST /users
        // =====================================================================

        router.add(
            http::verb::post,
            "/users",

            [](
                const Request& request,
                const auto&,
                const auto&)
            {
                Response response{
                    http::status::created,
                    request.version()
                };


                response.set(
                    http::field::content_type,
                    "text/plain"
                );


                response.body() =
                    "Created user\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );
    }
};

}