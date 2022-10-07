cmake_minimum_required(VERSION 3.20)

project(find)


# add_subdirectory(文本文件读写操作)

add_executable(${PROJECT_NAME} my_find_files2.cpp)
target_link_libraries(${PROJECT_NAME} boost_filesystem boost_system)