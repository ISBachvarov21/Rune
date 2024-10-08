
#include "server.hpp"
#include "routes/root.hpp"


void instantiateRoutes(CppHttp::Net::TcpListener &listener,
                       CppHttp::Net::Router &router) {
  router.DetatchAll();

	router.AddRoute("GET", "/", get_root);
	router.AddRoute("GET", "/test", get_test);
 
}
