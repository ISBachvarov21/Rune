#pragma once

#ifndef CPPHTTP
    #include <iostream>
    #include "ctre.hpp"
    #include "debug.hpp"
    #include "event.hpp"
    #include "responsetype.hpp"
    #include "router.hpp"
    #include "tcplistener.hpp"

    #define CPPHTTP
    #define HttpResponse std::tuple<CppHttp::Net::ResponseType, std::string, std::optional<std::vector<std::string>>>
    #define REQUEST CppHttp::Net::Request
    #define ROUTE_GET(route, func)\
      HttpResponse get_##func(REQUEST req)
    #define ROUTE_POST(route, func)\
      HttpResponse post_##func(REQUEST req)
    #define ROUTE_PUT(route, func)\
      HttpResponse put_##func(REQUEST req)
    #define ROUTE_DELETE(route, func)\
      HttpResponse delete_##func(REQUEST req)
#endif
