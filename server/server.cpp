#include "server.hpp"

HttpResponse root(REQUEST req) {
  return { CppHttp::Net::ResponseType::OK, "", {} };
}

void instantiateRoutes(CppHttp::Net::TcpListener* listener, CppHttp::Net::Router* router) { 
  listener->CreateSocket();

  listener->SetOnReceive([&](CppHttp::Net::Request req) {
      std::cout << "WHAT THE FUCK\n";
      router->Handle(req);
  });

  std::cout << "Creating routes" << std::endl;
  std::string path = "/";
  router->AddRoute("GET", path, root);
}
