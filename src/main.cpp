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

#include "../server/server.hpp"
#include "../dependencies/CppHttp/include/CppHttp.hpp"
#include <iostream>
#include <mutex>
#include <stop_token>
#include <sys/inotify.h>
#include <thread>
#include <unistd.h>
#include <vector>

const static std::mutex reloadMutex = std::mutex();

void server(std::stop_token stoken, bool *shoudlReload) {
 while (!stoken.stop_requested()) {
   CppHttp::Net::TcpListener *listener = getListener();
    while (!(*shoudlReload)) {
      listener->Listen("127.0.0.1", 8000, std::thread::hardware_concurrency());
      listener->Accept();
    }
    listener->Close();
    delete listener;
  }
}

int main() {
}

int mai() {
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

  // std::jthread serverThread(server);

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
