#pragma once
#include "../ORM/include/ScrollORM.hpp"

#include "../dependencies/CppHttp/include/CppHttp.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>
#include "Config.hpp"

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#include <sys/inotify.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dirent.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <filesystem>
#endif

typedef void (*instantiateRoutesFunc)(CppHttp::Net::TcpListener &,
                                      CppHttp::Net::Router &);

static std::mutex reloadMutex;

static int reloadCount = 0;

static instantiateRoutesFunc instantiateRoutes = nullptr;

const static std::string cppTemplate = R"(
#include "server.hpp"
{{HEADERS}}

void instantiateRoutes(CppHttp::Net::TcpListener &listener,
                       CppHttp::Net::Router &router) {
  router.DetatchAll();

{{ROUTES}} 
}
)";

struct alignas(64) DatabaseInfo {
  char dbConfig[512];
};

static json config;

void server(std::stop_token stoken, bool &shouldReload,
            std::mutex &reloadMutex);

void populateRoutes(std::vector<std::string> headers);

#if defined(__linux__) || defined(__APPLE__)
void *loadLibrary(const char *libPath);
#elif defined(_WIN32)
HMODULE loadLibrary(const char *libPath);
#endif


void watchFiles();
