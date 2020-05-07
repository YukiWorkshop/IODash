cmake_minimum_required(VERSION 3.2)
project(IODash)

set(CMAKE_CXX_STANDARD 17)

add_executable(IODash_Test
        test.cpp
        IODash.hpp IODash/SocketAddress.hpp IODash/File.hpp IODash/Socket.hpp IODash/EventLoop.hpp IODash/Serial.hpp IODash/Timer.hpp)

add_executable(IODash_Benchmark_HTTP Benchmarks/IODash_HTTP.cpp)
target_include_directories(IODash_Benchmark_HTTP PUBLIC .)

if (DEFINED BUILD_BENCHMARKS AND (${BUILD_BENCHMARKS}))
    add_executable(libuv_Benchmark_HTTP Benchmarks/libuv_HTTP.c)
    target_link_libraries(libuv_Benchmark_HTTP uv)

    add_executable(boost_Benchmark_HTTP Benchmarks/boost_HTTP.cpp)
    target_link_libraries(boost_Benchmark_HTTP boost_system pthread)
endif()

