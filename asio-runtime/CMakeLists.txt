
cmake_minimum_required(VERSION 3.20)

project(component_runtime
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/application")

# iocontext
add_executable(io_context src/asio/01_io_context/main.cpp)
target_link_libraries(io_context PRIVATE Boost::boost ws2_32 mswsock)

# executor
add_executable(executor src/asio/02_executor/main.cpp)
target_link_libraries(executor PRIVATE Boost::boost ws2_32 mswsock)

# post
add_executable(post src/asio/03_post/main.cpp)
target_link_libraries(post PRIVATE Boost::boost ws2_32 mswsock)

# work_guard
add_executable(work_guard src/asio/04_work_guard/main.cpp)
target_link_libraries(work_guard PRIVATE Boost::boost ws2_32 mswsock)

# thread_pool
add_executable(thread_pool src/asio/05_thread_pool/main.cpp)
target_link_libraries(thread_pool PRIVATE Boost::boost ws2_32 mswsock)
