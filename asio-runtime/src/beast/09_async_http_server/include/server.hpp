#pragma once

#include "listener.hpp"

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>

namespace net = boost::asio;

using tcp = net::ip::tcp;


// =============================================================================
// SERVER
// =============================================================================
//
// High-level server:
//
//     Server
//       │
//       ├── io_context
//       │
//       └── Listener
//              │
//              ├── Session
//              ├── Session
//              ├── Session
//              └── ...
//
// =============================================================================

class Server
{
private:

    net::io_context ioc;

    std::shared_ptr<Listener> listener;


public:

    explicit Server(
        unsigned short port
    )
    {
        listener =
            std::make_shared<Listener>(
                ioc,
                tcp::endpoint(
                    tcp::v4(),
                    port
                )
            );
    }


    void run()
    {
        listener->run();

        ioc.run();
    }
};