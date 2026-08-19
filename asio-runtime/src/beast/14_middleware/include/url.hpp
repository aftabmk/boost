#pragma once

#include "types.hpp"

#include <cstddef>
#include <string>
#include <string_view>

class Url
{
public:

    std::string path;

    QueryParameters query;


    // =========================================================================
    // PARSE
    // =========================================================================

    static Url parse(
        std::string_view target)
    {
        Url result;


        const std::size_t question =
            target.find('?');


        // ---------------------------------------------------------------------
        // No query parameters.
        // ---------------------------------------------------------------------

        if (question == std::string_view::npos)
        {
            result.path =
                std::string(target);

            return result;
        }


        // ---------------------------------------------------------------------
        // Path.
        // ---------------------------------------------------------------------

        result.path =
            std::string(
                target.substr(
                    0,
                    question
                )
            );


        // ---------------------------------------------------------------------
        // Query string.
        // ---------------------------------------------------------------------

        std::string_view query =
            target.substr(
                question + 1
            );


        while (!query.empty())
        {
            const std::size_t amp =
                query.find('&');


            std::string_view pair =
                query.substr(
                    0,
                    amp
                );


            const std::size_t equals =
                pair.find('=');


            if (equals == std::string_view::npos)
            {
                result.query.emplace(
                    std::string(pair),
                    ""
                );
            }
            else
            {
                result.query.emplace(
                    std::string(
                        pair.substr(
                            0,
                            equals
                        )
                    ),

                    std::string(
                        pair.substr(
                            equals + 1
                        )
                    )
                );
            }


            if (amp == std::string_view::npos)
                break;


            query =
                query.substr(
                    amp + 1
                );
        }


        return result;
    }
};