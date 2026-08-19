#pragma once

#include "types.hpp"
#include "router.hpp"

#include <boost/beast/http.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace http = boost::beast::http;


// =============================================================================
// APPLICATION
// =============================================================================

class Application
{
private:

    Router router;

    std::vector<Middleware> middlewares;


public:

    Application()
    {
        register_routes();

        register_middlewares();
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
                RequestContext& context,
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


                response.set(
                    "X-Request-ID",
                    context.request_id
                );


                response.body() =
                    "Hello from middleware server\n";


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
                RequestContext& context,
                const auto& parameters,
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


                response.set(
                    "X-Request-ID",
                    context.request_id
                );


                response.body() =
                    "User ID = " +
                    parameters.at("id") +
                    "\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // =====================================================================
        // GET /private
        // =====================================================================

        router.add(
            http::verb::get,
            "/private",

            [](
                const Request& request,
                RequestContext& context,
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


                response.set(
                    "X-Request-ID",
                    context.request_id
                );


                response.body() =
                    "You reached a protected resource\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );
    }


    // =========================================================================
    // REGISTER MIDDLEWARE
    // =========================================================================

    void register_middlewares()
    {
        // =====================================================================
        // MIDDLEWARE 1
        //
        // Request ID
        // =====================================================================

        middlewares.emplace_back(
            [](
                const Request& request,
                RequestContext& context,
                Next next)
            {
                (void)request;

                static std::atomic_uint64_t counter{
                    0
                };


                const std::uint64_t id =
                    ++counter;


                context.request_id =
                    "req-" +
                    std::to_string(id);


                Response response =
                    next();


                response.set(
                    "X-Request-ID",
                    context.request_id
                );


                return response;
            }
        );


        // =====================================================================
        // MIDDLEWARE 2
        //
        // Logging
        // =====================================================================

        middlewares.emplace_back(
            [](
                const Request& request,
                RequestContext& context,
                Next next)
            {
                std::cout
                    << "[REQUEST] "
                    << context.request_id
                    << " "
                    << request.method_string()
                    << " "
                    << request.target()
                    << '\n';


                Response response =
                    next();


                std::cout
                    << "[RESPONSE] "
                    << context.request_id
                    << " "
                    << response.result_int()
                    << '\n';


                return response;
            }
        );


        // =====================================================================
        // MIDDLEWARE 3
        //
        // Timing
        // =====================================================================

        middlewares.emplace_back(
            [](
                const Request& request,
                RequestContext& context,
                Next next)
            {
                (void)request;

                context.start_time =
                    std::chrono::steady_clock::now();


                Response response =
                    next();


                const auto end =
                    std::chrono::steady_clock::now();


                const auto elapsed =
                    std::chrono::duration_cast<
                        std::chrono::microseconds
                    >(
                        end -
                        context.start_time
                    );


                std::cout
                    << "[TIMING] "
                    << context.request_id
                    << " "
                    << elapsed.count()
                    << " us\n";


                response.set(
                    "X-Response-Time-us",
                    std::to_string(
                        elapsed.count()
                    )
                );


                return response;
            }
        );


        // =====================================================================
        // MIDDLEWARE 4
        //
        // Authentication
        // =====================================================================

        middlewares.emplace_back(
            [](
                const Request& request,
                RequestContext& context,
                Next next)
            {
                (void)context;

                const std::string target =
                    std::string(
                        request.target()
                    );


                if (target.starts_with(
                        "/private"))
                {
                    const auto authorization =
                        request[
                            http::field::authorization
                        ];


                    if (authorization !=
                        "Bearer secret")
                    {
                        Response response{
                            http::status::unauthorized,
                            request.version()
                        };


                        response.set(
                            http::field::content_type,
                            "text/plain"
                        );


                        response.set(
                            http::field::www_authenticate,
                            "Bearer"
                        );


                        response.body() =
                            "Unauthorized\n";


                        response.keep_alive(
                            request.keep_alive()
                        );


                        response.prepare_payload();

                        return response;
                    }
                }


                return next();
            }
        );
    }


public:

    // =========================================================================
    // HANDLE
    // =========================================================================

    Response handle(
        const Request& request)
    {
        RequestContext context;


        Next next =
            [&]()
            {
                return router.handle(
                    request,
                    context
                );
            };


        for (auto it = middlewares.rbegin();
             it != middlewares.rend();
             ++it)
        {
            Middleware middleware =
                *it;


            Next current =
                std::move(next);


            next =
                [middleware, current, &request, &context]()
                {
                    return middleware(
                        request,
                        context,
                        current
                    );
                };
        }


        return next();
    }
};