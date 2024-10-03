cmake_minimum_required(VERSION 3.8)

project(rune)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# include the directories include and dependencies/CppHttp/include
include_directories(include dependencies/CppHttp/include)

# recursiv  search for all files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(rune ${SOURCES})
