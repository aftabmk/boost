#pragma once

#include "types.hpp"

#include <boost/beast/http/verb.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


using Parameters =
    std::unordered_map<
        std::string,
        std::string
    >;


using Handler =
    std::function<
        Response(
            const Request&,
            const Parameters&,
            const Parameters&
        )
    >;


struct Route
{
    http::verb method;

    std::vector<std::string> segments;

    Handler handler;
};