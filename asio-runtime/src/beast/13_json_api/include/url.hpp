#pragma once

#include <string>
#include <string_view>
#include <unordered_map>


class Url
{
public:

    std::string path;

    std::unordered_map<
        std::string,
        std::string
    > query;


    static Url parse(
        std::string_view target)
    {
        Url result;

        const std::size_t question =
            target.find('?');


        // No query string.
        if (question == std::string_view::npos)
        {
            result.path =
                std::string(target);

            return result;
        }


        // Path.
        result.path =
            std::string(
                target.substr(
                    0,
                    question
                )
            );


        // Query.
        std::string_view query_string =
            target.substr(
                question + 1
            );


        while (!query_string.empty())
        {
            const std::size_t amp =
                query_string.find('&');


            std::string_view pair =
                query_string.substr(
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


            query_string =
                query_string.substr(
                    amp + 1
                );
        }


        return result;
    }
};