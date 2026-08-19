#pragma once

#include "response.hpp"
#include "router.hpp"

#include <boost/json.hpp>

#include <mutex>
#include <string>
#include <unordered_map>


namespace json = boost::json;


class Application
{
private:

    Router router;


    // =========================================================================
    // USER STORE
    // =========================================================================

    struct User
    {
        int id;

        std::string name;

        int age;
    };


    std::unordered_map<
        int,
        User
    > users;


    int next_id = 1;


    // -------------------------------------------------------------------------
    // Multiple sessions can execute on different io_context worker threads.
    //
    // Therefore access to users/next_id must be synchronized.
    // -------------------------------------------------------------------------

    mutable std::mutex users_mutex;


public:

    Application()
    {
        register_routes();
    }


    // =========================================================================
    // HANDLE
    // =========================================================================

    Response handle(
        const Request& request)
    {
        return router.handle(
            request
        );
    }


private:

    // =========================================================================
    // REGISTER ROUTES
    // =========================================================================

    void register_routes()
    {
        // =====================================================================
        // POST /users
        // =====================================================================

        router.add(
            http::verb::post,
            "/users",

            [this](
                const Request& request,
                const auto&,
                const auto&)
            {
                // -------------------------------------------------------------
                // Content-Type.
                // -------------------------------------------------------------

                auto content_type =
                    request[
                        http::field::content_type
                    ];


                if (content_type.empty())
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Content-Type must be application/json",
                        request.keep_alive()
                    );
                }


                // -------------------------------------------------------------
                // Parse JSON.
                // -------------------------------------------------------------

                json::value value;


                try
                {
                    value =
                        json::parse(
                            request.body()
                        );
                }
                catch (const std::exception&)
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Invalid JSON",
                        request.keep_alive()
                    );
                }


                // -------------------------------------------------------------
                // Root must be object.
                // -------------------------------------------------------------

                if (!value.is_object())
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "JSON body must be an object",
                        request.keep_alive()
                    );
                }


                const json::object& object =
                    value.as_object();


                // -------------------------------------------------------------
                // name
                // -------------------------------------------------------------

                auto name_it =
                    object.find("name");


                if (name_it == object.end() ||
                    !name_it->value().is_string())
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Field 'name' must be a string",
                        request.keep_alive()
                    );
                }


                // -------------------------------------------------------------
                // age
                // -------------------------------------------------------------

                auto age_it =
                    object.find("age");


                if (age_it == object.end() ||
                    !age_it->value().is_int64())
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Field 'age' must be an integer",
                        request.keep_alive()
                    );
                }


                // -------------------------------------------------------------
                // Create user.
                // -------------------------------------------------------------

                User user;


                {
                    std::lock_guard lock(
                        users_mutex
                    );


                    user.id =
                        next_id++;


                    user.name =
                        std::string(
                            name_it->value().as_string()
                        );


                    user.age =
                        static_cast<int>(
                            age_it->value().as_int64()
                        );


                    users.emplace(
                        user.id,
                        user
                    );
                }


                // -------------------------------------------------------------
                // Response.
                // -------------------------------------------------------------

                json::object body;

                body["id"] =
                    user.id;

                body["name"] =
                    user.name;

                body["age"] =
                    user.age;


                return json_response(
                    http::status::created,
                    request.version(),
                    body,
                    request.keep_alive()
                );
            }
        );


        // =====================================================================
        // GET /users/:id
        // =====================================================================

        router.add(
            http::verb::get,
            "/users/:id",

            [this](
                const Request& request,
                const auto& params,
                const auto&)
            {
                auto id_it =
                    params.find("id");


                if (id_it == params.end())
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Missing user id",
                        request.keep_alive()
                    );
                }


                int id;


                try
                {
                    id =
                        std::stoi(
                            id_it->second
                        );
                }
                catch (...)
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Invalid user id",
                        request.keep_alive()
                    );
                }


                User user;


                {
                    std::lock_guard lock(
                        users_mutex
                    );


                    auto user_it =
                        users.find(id);


                    if (user_it == users.end())
                    {
                        return json_error(
                            http::status::not_found,
                            request.version(),
                            "User not found",
                            request.keep_alive()
                        );
                    }


                    user =
                        user_it->second;
                }


                json::object body;

                body["id"] =
                    user.id;

                body["name"] =
                    user.name;

                body["age"] =
                    user.age;


                return json_response(
                    http::status::ok,
                    request.version(),
                    body,
                    request.keep_alive()
                );
            }
        );


        // =====================================================================
        // GET /users
        // =====================================================================

        router.add(
            http::verb::get,
            "/users",

            [this](
                const Request& request,
                const auto&,
                const auto&)
            {
                json::array array;


                {
                    std::lock_guard lock(
                        users_mutex
                    );


                    for (const auto& [id, user] : users)
                    {
                        (void)id;


                        json::object object;

                        object["id"] =
                            user.id;

                        object["name"] =
                            user.name;

                        object["age"] =
                            user.age;


                        array.emplace_back(
                            std::move(object)
                        );
                    }
                }


                return json_response(
                    http::status::ok,
                    request.version(),
                    array,
                    request.keep_alive()
                );
            }
        );


        // =====================================================================
        // GET /search
        // =====================================================================

        router.add(
            http::verb::get,
            "/search",

            [](
                const Request& request,
                const auto&,
                const auto& query)
            {
                auto it =
                    query.find("q");


                if (it == query.end())
                {
                    return json_error(
                        http::status::bad_request,
                        request.version(),
                        "Missing query parameter: q",
                        request.keep_alive()
                    );
                }


                json::object body;

                body["query"] =
                    it->second;


                return json_response(
                    http::status::ok,
                    request.version(),
                    body,
                    request.keep_alive()
                );
            }
        );
    }
};