#pragma once

#include "router.hpp"
#include "types.hpp"

#include <boost/beast/http.hpp>

#include <functional>
#include <string>
#include <thread>
#include <utility>

namespace http = boost::beast::http;


// =============================================================================
// APPLICATION
// =============================================================================
//
// Application
//      │
//      └── Router
//            │
//            ├── GET /
//            ├── GET /hello
//            ├── GET /thread
//            └── POST /echo
//
// Application is completely independent from networking.
//
// =============================================================================

class Application
{
private:

    Router router;


public:

    Application()
    {
        register_routes();
    }


    // =========================================================================
    // ROUTES
    // =========================================================================

    void register_routes()
    {
        // ---------------------------------------------------------------------
        // GET /
        // ---------------------------------------------------------------------

        router.add(
            http::verb::get,
            "/",

            [](const Request& request)
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
                    "Hello from Beast Router!\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // ---------------------------------------------------------------------
        // GET /hello
        // ---------------------------------------------------------------------

        router.add(
            http::verb::get,
            "/hello",

            [](const Request& request)
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
                    "Hello!\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // ---------------------------------------------------------------------
        // GET /thread
        // ---------------------------------------------------------------------

        router.add(
            http::verb::get,
            "/thread",

            [](const Request& request)
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
                    "Handled by thread: " +
                    std::to_string(
                        std::hash<std::thread::id>{}(
                            std::this_thread::get_id()
                        )
                    ) +
                    "\n";


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );


        // ---------------------------------------------------------------------
        // POST /echo
        // ---------------------------------------------------------------------

        router.add(
            http::verb::post,
            "/echo",

            [](const Request& request)
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
                    request.body();


                response.keep_alive(
                    request.keep_alive()
                );


                response.prepare_payload();

                return response;
            }
        );
    }


    // =========================================================================
    // HANDLE REQUEST
    // =========================================================================

    Response handle(
        const Request& request) const
    {
        return router.handle(request);
    }
};