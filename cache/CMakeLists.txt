cmake_minimum_required(VERSION 3.14)
project(cache_engine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(xxhash REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

# Header-only library
add_library(cache_engine INTERFACE)

# Compiler features
target_compile_features(cache_engine INTERFACE cxx_std_17)

# Compiler flags
target_compile_options(cache_engine INTERFACE 
    $<$<CONFIG:Release>:-O3 -march=native>
)

# Include directories
target_include_directories(cache_engine
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Dependencies
target_link_libraries(cache_engine
    INTERFACE
        xxHash::xxhash
        Boost::boost
        Threads::Threads
)

# Example
add_executable(example examples/main.cpp)
target_link_libraries(example PRIVATE cache_engine)

# Test
add_executable(test test/main.cpp)
target_link_libraries(test PRIVATE cache_engine)

# benchmark
add_executable(benchmark benchmark/main.cpp)
target_link_libraries(benchmark PRIVATE cache_engine)

# Install
install(DIRECTORY include/cache_engine
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)