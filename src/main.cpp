//#include "../dependencies/CppHttp/include/CppHttp.hpp"
//
//using namespace CppHttp;
//
//int main(void) {
//  CppHttp::Net::Router router;
//  CppHttp::Net::TcpListener listener;
//
//  listener.CreateSocket();
//  
//  listener.SetOnReceive([&](CppHttp::Net::Request req) {
//      router.Handle(req);
//  });
//
//  router.AddRoute("GET", "/", [](CppHttp::Net::Request req) -> HttpResponse {
//    return { CppHttp::Net::ResponseType::OK, "Hello, World!", {} };
//  });
//
//  listener.Listen("127.0.0.1", 8000, std::thread::hardware_concurrency());
//};





/*
  * TODO: need to add server function and server thread
  *       need to compile server files when files are modified
  *       need to reload server dll when files are modified
  *
  *       need to add a way to restart the server
  *       need to add a way to stop the server
  *       need to add a way to start the server
  *
  *       need to add server/routes header parsing
*/




#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <vector>

int main() {
  int fd = inotify_init();

  if (fd < 0) {
    std::cerr << "Failed to initialize inotify" << std::endl;
    return 1;
  }

  int wd = inotify_add_watch(fd, "./server/routes", IN_CREATE | IN_DELETE | IN_MODIFY);

  if (wd < 0) {
    std::cerr << "Failed to add watch" << std::endl;
    return 1;
  }

  char buffer[4096];

  std::vector<std::string> headers;

  bool shouldCompile = false;

  while (true) {
    shouldCompile = false;
    int length = read(fd, buffer, 4096);

    if (length < 0) {
      std::cerr << "Failed to read event" << std::endl;
      return 1;
    }

    int i = 0;
    
    while (i < length) {
      inotify_event *event = (inotify_event*) &buffer[i];

      if (event->mask & IN_MODIFY) {
        std::cout << "File modified" << std::endl;
        std::cout << "File name: " << event->name << std::endl;

        shouldCompile = true;
      }
      else if (event->mask & IN_CREATE) {
        std::cout << "File created" << std::endl;
        std::cout << "File name: " << event->name << std::endl;
        
        if (((std::string)event->name).find(".hpp") != std::string::npos) {
          headers.push_back(event->name);
        } 
      }
      else if (event->mask & IN_DELETE) {
        std::cout << "File deleted" << std::endl;
        std::cout << "File name: " << event->name << std::endl;
      
        if (((std::string)event->name).find(".hpp") != std::string::npos) {
          std::erase(headers, event->name);
        } 

        for (auto header : headers) {
          std::cout << header << std::endl;
        }
      }

      i += sizeof(inotify_event) + event->len;
    }

    if (shouldCompile) {
      std::cout << "[1;34m[*] Compiling...[0m]" << std::endl;

      // compile server files from CMakeLists.txt
      system("cmake -S server -B server/out && cmake --build server/out");

      // restart server 
    }
  }

  inotify_rm_watch(fd, wd);
  close(fd);
}
