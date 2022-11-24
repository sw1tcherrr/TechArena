cmake_minimum_required(VERSION 3.23)
project(geo)

set(CMAKE_CXX_STANDARD 20)

add_executable(geo main.cpp point.h metric.h polygon.h geo.h util.h box.h rtree.h)
