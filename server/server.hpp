#include "../dependencies/CppHttp/include/CppHttp.hpp"

#define EXPORT extern "C" __attribute__((visibility("default")))
#define REQUEST CppHttp::Net::Request

EXPORT void instantiateRoutes(CppHttp::Net::TcpListener &,
                              CppHttp::Net::Router &);
