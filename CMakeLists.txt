cmake_minimum_required(VERSION 3.2)
project(IODash)

set(CMAKE_CXX_STANDARD 17)

add_library(IODash INTERFACE)
target_include_directories(IODash INTERFACE .)

add_executable(IODash_Test test.cpp)
target_link_libraries(IODash_Test IODash)

add_executable(IODash_Benchmark_HTTP Benchmarks/IODash_HTTP.cpp)
target_link_libraries(IODash_Benchmark_HTTP IODash)

if (DEFINED BUILD_BENCHMARKS AND (${BUILD_BENCHMARKS}))
    add_executable(libuv_Benchmark_HTTP Benchmarks/libuv_HTTP.c)
    target_link_libraries(libuv_Benchmark_HTTP uv)

    add_executable(boost_Benchmark_HTTP Benchmarks/boost_HTTP.cpp)
    target_link_libraries(boost_Benchmark_HTTP boost_system pthread)
endif()

