#include "../dependencies/CppHttp/include/CppHttp.hpp"

const static std::string cppTemplate = R"(
#include "server.hpp"

void instantiateRoutes(CppHttp::Net::TcpListener *listener,
                       CppHttp::Net::Router *&router) {
  listener->CreateSocket();

  listener->SetOnReceive(
      [&](CppHttp::Net::Request req) { router->Handle(req); });

  {{ROUTES}}
}
)";

#define EXPORT extern "C" __attribute__((visibility("default")))
#define REQUEST CppHttp::Net::Request

EXPORT void instantiateRoutes(CppHttp::Net::TcpListener *,
                              CppHttp::Net::Router *&);
