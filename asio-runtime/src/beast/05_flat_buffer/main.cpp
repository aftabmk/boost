#include <boost/beast/core.hpp>
#include <boost/asio/buffer.hpp>

#include <cstring>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace net   = boost::asio;

int main()
{
    // -------------------------------------------------------------------------
    // 1. CREATE THE BUFFER
    //
    // flat_buffer owns a contiguous memory area.
    //
    // The buffer has two important regions:
    //
    //     [ readable bytes ][ writable space ]
    //              ↑
    //           data()
    //
    // `size()`     = readable bytes
    // `capacity()` = total allocated capacity
    //
    // The buffer grows automatically when more space is required.
    // -------------------------------------------------------------------------

    beast::flat_buffer buffer;

    std::cout << "Initial size     : "<< buffer.size()<< '\n';

    std::cout << "Initial capacity : "<< buffer.capacity()<< '\n';


    // -------------------------------------------------------------------------
    // 2. REQUEST WRITABLE MEMORY
    //
    // `prepare(n)` does NOT mean:
    //
    //     "the buffer now contains n bytes."
    //
    // It means:
    //
    //     "give me memory where I can write up to n bytes."
    //
    // The bytes are not considered readable until `commit()` is called.
    // -------------------------------------------------------------------------

    auto writable = buffer.prepare(16);


    // -------------------------------------------------------------------------
    // 3. SIMULATE A SOCKET READ
    //
    // Normally this would be:
    //
    //     socket.read_some(writable);
    //
    // Here we simply copy data into the writable region to demonstrate
    // the buffer protocol without requiring a network connection.
    // -------------------------------------------------------------------------

    const std::string incoming = "Hello, Beast!";

    std::memcpy(writable.data(),incoming.data(),incoming.size());


    // -------------------------------------------------------------------------
    // 4. COMMIT THE RECEIVED BYTES
    //
    // Before commit():
    //
    //     buffer knows that space exists,
    //     but it does NOT consider those bytes readable.
    //
    // After commit(n):
    //
    //     those n bytes become part of the readable region.
    // -------------------------------------------------------------------------

    buffer.commit(incoming.size());


    std::cout << "\nAfter receiving data:\n";

    std::cout << "Size     : "<< buffer.size()<< '\n';

    std::cout << "Capacity : "<< buffer.capacity()<< '\n';


    // -------------------------------------------------------------------------
    // 5. READ THE DATA
    //
    // `data()` returns the readable portion of the buffer.
    //
    // Beast and Asio can use this directly as a ConstBufferSequence.
    //
    // We use a string_view here only to demonstrate the contents.
    // -------------------------------------------------------------------------

    auto readable = buffer.data();

    std::string received(
        static_cast<const char*>(readable.data()),
        readable.size()
    );

    std::cout << "Data     : "<< received<< '\n';


    // -------------------------------------------------------------------------
    // 6. CONSUME DATA
    //
    // `consume(n)` removes n bytes from the FRONT of the readable region.
    //
    // This is extremely important for protocol parsing.
    //
    // Example:
    //
    //     [HTTP request][next message]
    //      ^^^^^^^^^^^^
    //
    // Once the first message has been processed:
    //
    //     consume(first_message_size)
    //
    // leaves:
    //
    //     [next message]
    // -------------------------------------------------------------------------

    buffer.consume(7);


    std::cout << "\nAfter consume(7):\n";

    std::cout << "Size : "<< buffer.size()<< '\n';


    readable = buffer.data();

    std::string remaining(
        static_cast<const char*>(readable.data()),
        readable.size()
    );

    std::cout << "Data : "<< remaining<< '\n';


    // -------------------------------------------------------------------------
    // 7. ADD MORE DATA
    //
    // We can continue receiving data into the same buffer.
    //
    // This is how a buffer can survive across multiple socket reads.
    // -------------------------------------------------------------------------

    const std::string second_chunk = " Network";

    auto writable2 = buffer.prepare(second_chunk.size());

    std::memcpy(
        writable2.data(),
        second_chunk.data(),
        second_chunk.size()
    );

    buffer.commit(second_chunk.size());


    // -------------------------------------------------------------------------
    // 8. READ THE FINAL CONTENT
    // -------------------------------------------------------------------------

    readable = buffer.data();

    std::string final_data(
        static_cast<const char*>(readable.data()),
        readable.size()
    );

    std::cout << "\nAfter receiving another chunk:\n";

    std::cout << "Data : "<< final_data<< '\n';


    // -------------------------------------------------------------------------
    // 9. BUFFER LIMIT
    //
    // In production servers you should usually impose a maximum buffer size.
    //
    // Otherwise a malicious/slow client could continuously send data and
    // force the server to allocate increasing amounts of memory.
    //
    // Beast supports this through `max_size()`.
    // -------------------------------------------------------------------------

    beast::flat_buffer limited_buffer;

    limited_buffer.max_size(1024);

    std::cout << "\nMaximum buffer size: "<< limited_buffer.max_size()<< " bytes\n";


    // -------------------------------------------------------------------------
    // IMPORTANT CONCEPT
    //
    // The three operations to remember are:
    //
    //     prepare()
    //         ↓
    //     write into writable memory
    //         ↓
    //     commit()
    //         ↓
    //     readable data
    //         ↓
    //     parser consumes it
    //         ↓
    //     consume()
    //
    // This is the fundamental buffer lifecycle.
    // -------------------------------------------------------------------------

    return 0;
}