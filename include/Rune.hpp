#include "../dependencies/CppHttp/include/CppHttp.hpp"
#include <chrono>
#include <dlfcn.h>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <sys/inotify.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <dirent.h>

typedef void (*instantiateRoutesFunc)(CppHttp::Net::TcpListener &,
                                      CppHttp::Net::Router &);
static std::mutex reloadMutex;

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

void server(std::stop_token stoken, bool &shouldReload,
            std::mutex &reloadMutex);

void populateRoutes(std::vector<std::string> headers);

void *loadLibrary(const char *libPath);

void watchFiles();
