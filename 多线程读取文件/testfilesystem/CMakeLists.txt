cmake_minimum_required(VERSION 3.20)
project(test_file_system)

# set ( MY_CXX_FLAGS "-finput-charset  -fexecute-charset" )
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_CXX_FLAGS}" )
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} boost_filesystem boost_system)
