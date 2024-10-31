
#pragma once
#include "Database.hpp"
#include "../dependencies/CppHttp/include/CppHttp.hpp"
#include "routes/root.hpp"


#if defined(__linux__) || defined(__APPLE__)
#define EXPORT extern "C" __attribute__((visibility("default")))
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <string>
#elif defined(_WIN32)
#define EXPORT extern "C" __declspec(dllexport)
#endif

#define REQUEST CppHttp::Net::Request

EXPORT void instantiateRoutes(CppHttp::Net::TcpListener &,
                       CppHttp::Net::Router &router);
