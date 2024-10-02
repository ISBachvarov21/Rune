#include "../dependencies/CppHttp/include/CppHttp.hpp"

const static std::string cppTemplate = R"(
#include "server.hpp"

CppHttp::Net::TcpListener* getListener() {
CppHttp::Net::Router* router = new CppHttp::Net::Router();
  CppHttp::Net::TcpListener* listener = new CppHttp::Net::TcpListener();
  
  listener->CreateSocket();

  listener->SetOnReceive([&](CppHttp::Net::Request req) {
    router->Handle(req);
  });

  {{routes}}

  return listener;
}
)";

#define EXPORT extern "C" __attribute__((visibility("default")))
#define REQUEST CppHttp::Net::Request

EXPORT void instantiateRoutes(CppHttp::Net::TcpListener*, CppHttp::Net::Router*);

HttpResponse root(REQUEST req); 
