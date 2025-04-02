cmake_minimum_required(VERSION 3.8)

project(server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")

find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(../dependencies/soci/include)
include_directories("../dependencies/CppHttp/include")

add_library(server SHARED server.cpp Database.cpp)

target_link_libraries(server PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/soci/lib/libsoci_postgresql.a
  ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/soci/lib/libsoci_core.a
  ${PostgreSQL_LIBRARIES}
  ${OPENSSL_LIBRARIES}
)

set_target_properties(server PROPERTIES OUTPUT_NAME "server")
