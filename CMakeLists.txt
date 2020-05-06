cmake_minimum_required(VERSION 3.2)
project(IODash)

set(CMAKE_CXX_STANDARD 17)

add_executable(IODash_Test
        test.cpp
        IODash.hpp IODash/SocketAddress.hpp IODash/File.hpp IODash/Socket.hpp IODash/EventLoop.hpp IODash/Serial.hpp IODash/Timer.hpp)
