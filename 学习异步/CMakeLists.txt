cmake_minimum_required(VERSION 3.20)
project(test1)

set(CMAKE_CXX_STANDARD 20)
# set ( MY_CXX_FLAGS "-fcoroutines" )
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_CXX_FLAGS}" )
# add_executable(test1 main.cpp)
add_executable(test1 async_http_server.cpp)

