cmake_minimum_required(VERSION 3.8)

project(server)

include_directories("../dependencies/CppHttp/include")

add_library(server server.cpp)
