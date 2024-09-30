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

CppHttp::Net::TcpListener* getListener();
