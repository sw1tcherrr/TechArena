cmake_minimum_required(VERSION 3.23)
project(geo)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost REQUIRED)

add_executable(geo main.cpp)

target_link_libraries(geo PRIVATE Boost::boost)
