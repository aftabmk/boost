cmake_minimum_required(VERSION 3.20)

project(component_runtime
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ==============================================================================
# BOOST
# ==============================================================================

find_package(Boost REQUIRED)


# ==============================================================================
# GLOBAL OUTPUT DIRECTORY
# ==============================================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/application"
)


# ==============================================================================
# APPLICATION HELPER
# ==============================================================================
#
# Usage:
#
# add_app_executable(
#     TARGET_NAME
#     TYPE
#     SOURCE_FILE
# )
#
# Example:
#
# add_app_executable(
#     async_http_server
#     beast
#     src/beast/09_async_http_server/main.cpp
# )
#
# If the application contains:
#
#     src/beast/09_async_http_server/include/
#
# it is automatically added to the include path.
#
# ==============================================================================

function(add_app_executable TARGET_NAME TYPE SOURCE_FILE)

    # --------------------------------------------------------------------------
    # Create executable
    # --------------------------------------------------------------------------
    # ${ARGN} collects all extra arguments passed beyond the 3 named parameters
    add_executable(
        ${TARGET_NAME}
        ${SOURCE_FILE}
    )


    # --------------------------------------------------------------------------
    # Common libraries
    # --------------------------------------------------------------------------

    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            Boost::boost
            Boost::json
            ws2_32
            mswsock
    )


    # --------------------------------------------------------------------------
    # Application-specific include directory
    # --------------------------------------------------------------------------
    #
    # SOURCE_FILE:
    #
    #     src/beast/09_async_http_server/main.cpp
    #
    # SOURCE_DIR becomes:
    #
    #     src/beast/09_async_http_server
    #
    # Therefore:
    #
    #     src/beast/09_async_http_server/include
    #
    # can contain server.hpp, listener.hpp, session.hpp.
    #
    # --------------------------------------------------------------------------

    get_filename_component(
        SOURCE_DIR
        "${SOURCE_FILE}"
        DIRECTORY
    )

    set(APP_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_DIR}/include"
    )


    if(EXISTS "${APP_INCLUDE_DIR}")

        target_include_directories(
            ${TARGET_NAME}
            PRIVATE
                "${APP_INCLUDE_DIR}"
        )

    endif()


    # --------------------------------------------------------------------------
    # Output directory
    # --------------------------------------------------------------------------

    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY
                "${CMAKE_BINARY_DIR}/../application/${TYPE}"
    )

endfunction()


# ==============================================================================
# ASIO APPLICATIONS
# ==============================================================================

add_app_executable(io_context               asio        src/asio/01_io_context/main.cpp)
add_app_executable(executor                 asio        src/asio/02_executor/main.cpp)
add_app_executable(post                     asio        src/asio/03_post/main.cpp)
add_app_executable(work_guard               asio        src/asio/04_work_guard/main.cpp)
add_app_executable(thread_pool              asio        src/asio/05_thread_pool/main.cpp)
add_app_executable(strand                   asio        src/asio/06_strand/main.cpp)
add_app_executable(timer                    asio        src/asio/07_timer/main.cpp)
add_app_executable(async_operation          asio        src/asio/08_async_operation/main.cpp)
add_app_executable(awaitable                asio        src/asio/09_awaitable/main.cpp)
add_app_executable(co_spawn                 asio        src/asio/10_co_spawn/main.cpp)
add_app_executable(co_await                 asio        src/asio/11_co_await/main.cpp)
add_app_executable(use_awaitable            asio        src/asio/12_use_awaitable/main.cpp)
add_app_executable(cancellation             asio        src/asio/13_cancellation/main.cpp)
add_app_executable(coroutine_concurrency    asio        src/asio/14_coroutine_concurrency/main.cpp)

# ==============================================================================
# BEAST APPLICATIONS
# ==============================================================================

 add_app_executable(http_message            beast       src/beast/01_http_message/main.cpp)
 add_app_executable(async_http_server       beast       src/beast/09_async_http_server/main.cpp)
 add_app_executable(multithreaded_server    beast       src/beast/10_multithreaded_server/main.cpp)
 add_app_executable(router                  beast       src/beast/11_router/main.cpp)
 add_app_executable(dynamic_router          beast       src/beast/12_dynamic_router/main.cpp)
 add_app_executable(json_api                beast       src/beast/13_json_api/main.cpp)
 add_app_executable(middleware              beast       src/beast/14_middleware/main.cpp)

# ==============================================================================
# LOCKFREE APPLICATIONS
# ==============================================================================
 add_app_executable(spsc_queue              lockfree    src/lockfree/01_spsc_queue/main.cpp)
 add_app_executable(mpmc_queue              lockfree    src/lockfree/02_mpmc_queue/main.cpp)
 add_app_executable(stack                   lockfree    src/lockfree/03_stack/main.cpp)
# ==============================================================================
# MEMORY ORDER APPLICATIONS
# ==============================================================================
 add_app_executable(relaxed                 memory_order    src/memory_order/01_relaxed/main.cpp)
 add_app_executable(release                 memory_order    src/memory_order/02_release_acquire/main.cpp)
 add_app_executable(compare_swap            memory_order    src/memory_order/03_cas/main.cpp)
 add_app_executable(compare_swap_loop       memory_order    src/memory_order/04_cas_loop/main.cpp)
 add_app_executable(acquire_release         memory_order    src/memory_order/05_acq_rel/main.cpp)
# ==============================================================================
# CONTAINER APPLICATIONS
# ==============================================================================
# ==============================================================================
# GRAPH APPLICATIONS
# ==============================================================================
# ==============================================================================
# INTERPROCESS APPLICATIONS
# ==============================================================================
# ==============================================================================
# JSON APPLICATIONS
# ==============================================================================
# ==============================================================================
# LOG APPLICATIONS
# ==============================================================================
# ==============================================================================
# POOL APPLICATIONS
# ==============================================================================