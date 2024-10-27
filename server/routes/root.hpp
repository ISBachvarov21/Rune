#include "../../dependencies/CppHttp/include/CppHttp.hpp"

ROUTE_GET("/", root) {
  return {(CppHttp::Net::ResponseType)200, "Rune Project", {}};    
}
