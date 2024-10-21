#include "../dependencies/CppHttp/include/CppHttp.hpp"

#if defined(__linux__) || defined(__APPLE__)
#define EXPORT extern "C" __attribute__((visibility("default")))
#elif defined(_WIN32)
#define EXPORT extern "C" __declspec(dllexport)
#endif

#define REQUEST CppHttp::Net::Request

EXPORT void instantiateRoutes(CppHttp::Net::TcpListener &,
                              CppHttp::Net::Router &);
