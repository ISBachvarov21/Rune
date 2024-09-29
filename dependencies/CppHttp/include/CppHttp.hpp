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
#endif