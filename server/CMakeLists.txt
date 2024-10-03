cmake_minimum_required(VERSION 3.8)

project(server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

include_directories("../dependencies/CppHttp/include")

add_library(server SHARED server.cpp)
