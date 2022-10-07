cmake_minimum_required(VERSION 3.20)

project(read)


# add_subdirectory(testfilesystem)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} boost_filesystem boost_system)

