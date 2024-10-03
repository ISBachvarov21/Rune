#include "server.hpp"

void instantiateRoutes(CppHttp::Net::TcpListener *listener,
                       CppHttp::Net::Router *&router) {
  listener->CreateSocket();

  listener->SetOnReceive(
      [&](CppHttp::Net::Request req) { router->Handle(req); });

  router->AddRoute("GET", "/", [](REQUEST req) -> HttpResponse {
    return { CppHttp::Net::ResponseType::OK, "test", {} };
  });
}
