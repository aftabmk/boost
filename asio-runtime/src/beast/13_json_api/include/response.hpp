#pragma once

#include "types.hpp"

#include <boost/json.hpp>

#include <string_view>


namespace json = boost::json;


// =============================================================================
// JSON RESPONSE
// =============================================================================

inline Response json_response(
    http::status status,
    unsigned version,
    const json::value& value,
    bool keep_alive)
{
    Response response{
        status,
        version
    };


    response.set(
        http::field::content_type,
        "application/json"
    );


    response.body() =
        json::serialize(value);


    response.keep_alive(
        keep_alive
    );


    response.prepare_payload();

    return response;
}


// =============================================================================
// JSON ERROR
// =============================================================================

inline Response json_error(
    http::status status,
    unsigned version,
    std::string_view message,
    bool keep_alive)
{
    json::object object;

    object["error"] =
        message;


    return json_response(
        status,
        version,
        object,
        keep_alive
    );
}