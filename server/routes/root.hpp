#include "../../dependencies/CppHttp/include/CppHttp.hpp"

ROUTE_GET("/", root) {
  return {(CppHttp::Net::ResponseType)200, "Hello, World!", {}};    
}

ROUTE_GET("/test", test) {
  return {(CppHttp::Net::ResponseType)404, "Hello, Test!", {}};    
}
