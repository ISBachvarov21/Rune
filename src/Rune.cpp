#include "../include/Rune.hpp"

void server(std::stop_token stoken, bool &shouldReload,
            std::mutex &reloadMutex) {
  CppHttp::Net::Router router;
  CppHttp::Net::TcpListener listener;

  listener.CreateSocket();
  listener.SetOnReceive([&](CppHttp::Net::Request req) { router.Handle(req); });

  listener.Bind("127.0.0.1", 8000, std::thread::hardware_concurrency());
  listener.Listen("127.0.0.1", 8000, std::thread::hardware_concurrency());

  while (!stoken.stop_requested()) {
    if (instantiateRoutes) {
      instantiateRoutes(listener, router);
    }

    while (!shouldReload) {
      listener.Accept();
    }

    {
      std::lock_guard<std::mutex> lock(reloadMutex);
      shouldReload = false;
      std::cout << "\033[1;34m[*] Server reloaded\033[0m" << std::endl;
    }
  }

  listener.Close();
}

void populateRoutes(std::vector<std::string> headers) {
  std::string routesValue = "";
  std::string headersValue = "";

  for (auto header : headers) {
    headersValue += "#include \"routes/" + header + "\"\n";

    std::string path = "./server/routes/" + header;
    std::ifstream file(path); 
    std::string line;

    while (std::getline(file, line)) {
      size_t getIndex = line.find("ROUTE_GET");
      size_t postIndex = line.find("ROUTE_POST");
      size_t putIndex = line.find("ROUTE_PUT");
      size_t deleteIndex = line.find("ROUTE_DELETE");
      size_t index = getIndex != std::string::npos      ? getIndex
                     : postIndex != std::string::npos   ? postIndex
                     : putIndex != std::string::npos    ? putIndex
                     : deleteIndex != std::string::npos ? deleteIndex
                                                        : std::string::npos;

      if (index != std::string::npos) {
        std::string method = getIndex != std::string::npos      ? "GET"
                             : postIndex != std::string::npos   ? "POST"
                             : putIndex != std::string::npos    ? "PUT"
                             : deleteIndex != std::string::npos ? "DELETE"
                                                                : "";
        size_t pathIndex = line.find("\"", index);
        size_t pathEndIndex = line.find("\"", pathIndex + 1);

        std::string route =
            line.substr(pathIndex + 1, pathEndIndex - pathIndex - 1);
        std::cout << route << std::endl;

        size_t funcNameIndex = line.find(",", pathEndIndex);
        size_t funcNameEndIndex = line.find(")", funcNameIndex);

        std::string funcName = line.substr(
            funcNameIndex + 2, funcNameEndIndex - funcNameIndex - 2);
        std::cout << funcName << std::endl;

        std::string methodCopy = method;
        std::transform(method.begin(), method.end(), method.begin(), ::tolower);

        routesValue += "\trouter.AddRoute(\"" + methodCopy + "\", \"" + route +
                       "\", " + method + "_" + funcName + ");\n";
      }
    }
  }

  std::string templateCopy = cppTemplate;
  templateCopy.replace(templateCopy.find("{{ROUTES}}"), 10, routesValue);
  templateCopy.replace(templateCopy.find("{{HEADERS}}"), 11, headersValue);

  std::ofstream file("./server/server.cpp", std::ios::trunc);
  file << templateCopy;

  file.close();
}

void *loadLibrary(const char *libPath) {
  void *libHandle = dlmopen(LM_ID_NEWLM, libPath, RTLD_NOW | RTLD_LOCAL);
  if (!libHandle) {
    std::cerr << "Failed to load library: " << dlerror() << std::endl;
    return nullptr;
  }
  return libHandle;
}

void watchFiles() {
  int fd = inotify_init();
  if (fd < 0) {
    std::cerr << "Failed to initialize inotify" << std::endl;
    return;
  }

  int wd = inotify_add_watch(fd, "./server/routes",
                             IN_CREATE | IN_DELETE | IN_MODIFY);
  if (wd < 0) {
    std::cerr << "Failed to add watch" << std::endl;
    return;
  }

  char buffer[4096];
  std::vector<std::string> headers;

  DIR *dir;
  dirent *ent;
  if ((dir = opendir("./server/routes")) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_REG) {
        headers.push_back(ent->d_name);
      }
    }
    closedir(dir);
  }

  bool shouldCompile = false;
  bool shouldReload = false;

  populateRoutes(headers);
  system("cmake -S server -B server/out && cmake --build server/out");

  void *serverLib = loadLibrary("./server/out/libserver.so");
  if (!serverLib) {
    return;
  }

  instantiateRoutes =
      (instantiateRoutesFunc)dlsym(serverLib, "instantiateRoutes");

  char *error = dlerror();
  if (error) {
    std::cerr << "Failed to load instantiateRoutes: " << error << std::endl;
    return;
  }

  std::jthread serverThread(server, std::ref(shouldReload),
                            std::ref(reloadMutex));

  while (true) {
    shouldCompile = false;
    int length = read(fd, buffer, sizeof(buffer));

    if (length < 0) {
      std::cerr << "Failed to read event" << std::endl;
      return;
    }

    int i = 0;
    while (i < length) {
      inotify_event *event = (inotify_event *)&buffer[i];

      if (event->mask & IN_MODIFY) {
        shouldCompile = true;
      } else if (event->mask & IN_CREATE) {
        if (((std::string)event->name).find(".hpp") != std::string::npos &&
            std::find(headers.begin(), headers.end(), event->name) ==
                headers.end()) {
          headers.push_back(event->name);
        }
      } else if (event->mask & IN_DELETE) {
        if (((std::string)event->name).find(".hpp") != std::string::npos) {
          std::erase(headers, event->name);
        }
      }

      i += sizeof(inotify_event) + event->len;
    }

    if (shouldCompile) {
      std::cout << "\033[1;34m[*] Compiling...\033[0m" << std::endl;

      if (serverLib) {
        dlclose(serverLib);
        serverLib = nullptr;
        instantiateRoutes = nullptr;
      }

      populateRoutes(headers);
      system("rm -rf server/out");
      system("cmake -S server -B server/out && cmake --build server/out");

      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      serverLib = loadLibrary("./server/out/libserver.so");
      if (!serverLib) {
        continue;
      }

      instantiateRoutes =
          (instantiateRoutesFunc)dlsym(serverLib, "instantiateRoutes");

      error = dlerror();
      if (error) {
        std::cerr << "Failed to load instantiateRoutes: " << error << std::endl;
        continue;
      }

      shouldCompile = false;
      {
        std::lock_guard<std::mutex> lock(reloadMutex);
        shouldReload = true;
      }
    }
  }

  inotify_rm_watch(fd, wd);
  close(fd);
}
