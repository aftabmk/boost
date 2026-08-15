cmake_minimum_required(VERSION 3.20)

project(component_runtime
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

# Global base output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/application")

# --- Custom Helper Function to Prevent Copy-Paste Bugs ---
function(add_app_executable TARGET_NAME TYPE SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE Boost::boost ws2_32 mswsock)
    
    # Places the binary into application/asio or application/beast dynamically
    set_target_properties(${TARGET_NAME} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../application/${TYPE}"
    )
endfunction()

# ==============================================================================
# ASIO Applications
# ==============================================================================
add_app_executable(io_context               asio src/asio/01_io_context/main.cpp)
add_app_executable(executor                 asio src/asio/02_executor/main.cpp)
add_app_executable(post                     asio src/asio/03_post/main.cpp)
add_app_executable(work_guard               asio src/asio/04_work_guard/main.cpp)
add_app_executable(thread_pool              asio src/asio/05_thread_pool/main.cpp)
add_app_executable(strand                   asio src/asio/06_strand/main.cpp)
add_app_executable(timer                    asio src/asio/07_timer/main.cpp)
add_app_executable(async_operation          asio src/asio/08_async_operation/main.cpp)
add_app_executable(awaitable                asio src/asio/09_awaitable/main.cpp)
add_app_executable(co_spawn                 asio src/asio/10_co_spawn/main.cpp)
add_app_executable(co_await                 asio src/asio/11_co_await/main.cpp)
add_app_executable(use_awaitable            asio src/asio/12_use_awaitable/main.cpp)
add_app_executable(cancellation             asio src/asio/13_cancellation/main.cpp)
add_app_executable(coroutine_concurrency    asio src/asio/14_coroutine_concurrency/main.cpp)

# ==============================================================================
# BEAST Applications
# ==============================================================================
add_app_executable(http_message             beast src/beast/01_http_message/main.cpp)
