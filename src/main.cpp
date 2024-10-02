/*
 * TODO: need to compile server files when files are modified
 *       need to reload server dll when files are modified
 *
 *       need to add server/routes header parsing
*/

/*
 * INFO: {{routes}} in cppTemplate is at index 323 and ends at index 333
 *
 *
*/

#include "../dependencies/CppHttp/include/CppHttp.hpp"
#include <iostream>
#include <mutex>
#include <stop_token>
#include <sys/inotify.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <dlfcn.h>

typedef void (*instantiateRoutesFunc)(CppHttp::Net::TcpListener*, CppHttp::Net::Router*);
const static std::mutex reloadMutex = std::mutex();

void server(std::stop_token stoken, bool *shouldReload, instantiateRoutesFunc* listenerPtr) {
 while (!stoken.stop_requested()) {
    CppHttp::Net::Router* router = new CppHttp::Net::Router();
    CppHttp::Net::TcpListener* listener = new CppHttp::Net::TcpListener();
  
    (*listenerPtr)(listener, router);

    listener->Bind("127.0.0.1", 8000, std::thread::hardware_concurrency());
    listener->Listen("127.0.0.1", 8000, std::thread::hardware_concurrency());
   
    while (!(*shouldReload)) {
      listener->Accept();
    }
    
    listener->Close();
    delete listener;
    delete router;
  }
}

int main() {
  int fd = inotify_init();

  if (fd < 0) {
    std::cerr << "Failed to initialize inotify" << std::endl;
    return 1;
  }

  int wd = inotify_add_watch(fd, "./server/routes",
                             IN_CREATE | IN_DELETE | IN_MODIFY);

  if (wd < 0) {
    std::cerr << "Failed to add watch" << std::endl;
    return 1;
  }

  char buffer[4096];

  std::vector<std::string> headers;

  bool shouldCompile = false;

  void* dll = dlopen("./server/out/libserver.so", RTLD_NOW);
  
  if (!dll) {
    std::cout << "Error loading dll";
    return 1;
  } 


  instantiateRoutesFunc listenerPtr = (instantiateRoutesFunc)dlsym(dll, "instantiateRoutes");
  char* error;
  error = dlerror();

  if (error != NULL) {
    std::cout << "Failed to load getListener\n";
  }
  std::jthread serverThread(server, &shouldCompile, &listenerPtr);
  
  while (true) {
    shouldCompile = false;
    int length = read(fd, buffer, 4096);

    if (length < 0) {
      std::cerr << "Failed to read event" << std::endl;
      return 1;
    }

    int i = 0;
    
    while (i < length) {
      inotify_event *event = (inotify_event *)&buffer[i];

      if (event->mask & IN_MODIFY) {
        std::cout << "File modified" << std::endl;
        std::cout << "File name: " << event->name << std::endl;

        shouldCompile = true;
      } else if (event->mask & IN_CREATE) {
        std::cout << "File created" << std::endl;
        std::cout << "File name: " << event->name << std::endl;

        if (((std::string)event->name).find(".hpp") != std::string::npos) {
          headers.push_back(event->name);
        }
      } else if (event->mask & IN_DELETE) {
        std::cout << "File deleted" << std::endl;
        std::cout << "File name: " << event->name << std::endl;

        if (((std::string)event->name).find(".hpp") != std::string::npos) {
          std::erase(headers, event->name);
        }

      }

      i += sizeof(inotify_event) + event->len;
    }

    if (shouldCompile) {
      std::cout << "[1;34m[*] Compiling...[0m]" << std::endl;

      // compile server files from CMakeLists.txt
      system("cmake -S server -B server/out && cmake --build server/out");

      // restart server

      dll = dlopen("./server/out/libserver.so", RTLD_NOW);
      listenerPtr = (instantiateRoutesFunc)dlsym(dll, "instantiateRoutes");
      shouldCompile = false;
    }
  }


  inotify_rm_watch(fd, wd);
  close(fd);
}
