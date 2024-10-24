#include "../include/Rune.hpp"

// TODO: replace literal string values with config values (continue from line
// 168)

void loadConfig() {
  std::ifstream file("rune.json");
  if (!file) {
    std::ofstream newFile("rune.json");

    std::cout << "\033[1;31m[-] Configuration file not found\033[0m"
              << std::endl;
    std::cout << "\033[1;31m[-] Creating new configuration file..\033[0m"
              << std::endl;
    // Default configuration
    newFile << "{\n"
            << "\t\"port\": 8000,\n"
            << "\t\"host\": \"127.0.0.1\",\n"
            << "\t\"server_location\": \"./server\",\n"
            << "\t\"endpoint_folder\": \"routes\"\n"
            << "}";
    newFile.close();
    file.open("rune.json");
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      (std::istreambuf_iterator<char>()));
  config = json::parse(content);
  if (config.is_discarded()) {
    std::cerr << "Failed to parse configuration" << std::endl;
    return;
  }
  if (!config.contains("port") || !config["port"].is_number_integer()) {
    std::cerr << "Invalid port" << std::endl;
    return;
  }
  if (!config.contains("host") || !config["host"].is_string()) {
    std::cerr << "Invalid host" << std::endl;
    return;
  }
  if (!config.contains("server_location") ||
      !config["server_location"].is_string()) {
    std::cerr << "Invalid server location" << std::endl;
    return;
  }
  if (!config.contains("endpoint_folder") ||
      !config["endpoint_folder"].is_string()) {
    std::cerr << "Invalid endpoint folder" << std::endl;
    return;
  }

  file.close();
}

void server(std::stop_token stoken, bool &shouldReload,
            std::mutex &reloadMutex) {
  CppHttp::Net::Router router;
  CppHttp::Net::TcpListener listener;

  listener.CreateSocket();
  listener.SetOnReceive([&](CppHttp::Net::Request req) { router.Handle(req); });

  listener.Bind(config["host"].get<std::string>().c_str(),
                config["port"].get<int>(), std::thread::hardware_concurrency());
  listener.Listen(config["host"].get<std::string>().c_str(),
                  config["port"].get<int>(),
                  std::thread::hardware_concurrency());

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

void instantiateDB() {
  // connection url format:
  // "<provider>://<user>:<password>@<host>:<port>/<database>" NOTE: only
  // only supports PostgreSQL for now
  // TODO: add support for other database providers
  
  if (config.contains("database")) {
    if (!config["database"].contains("connection_url")) {
      std::cerr << "\033[1;31m[-] Database connection url not found\033[0m"
                << std::endl;
    }
    std::string connectionUrl =
        config["database"]["connection_url"].get<std::string>();

    size_t providerSeparation = connectionUrl.find("://");

    if (providerSeparation == std::string::npos) {
      std::cerr << "\033[1;31m[-] Database provider not found\033[0m"
                << std::endl;
      return;
    }

    std::string provider = connectionUrl.substr(0, providerSeparation);

    if (provider != "postgresql") {
      std::cerr << "\033[1;31m[-] Database provider not supported\033[0m"
                << std::endl;
      return;
    }

    size_t userSeparation = connectionUrl.find(":", providerSeparation + 3);

    if (userSeparation == std::string::npos) {
      std::cerr << "\033[1;31m[-] Database user not found\033[0m" << std::endl;
      return;
    }

    size_t passwordSeparation = connectionUrl.find("@", userSeparation + 1);

    if (passwordSeparation == std::string::npos) {
      std::cerr << "\033[1;31m[-] Database password not found\033[0m"
                << std::endl;
      return;
    }

    std::string user = connectionUrl.substr(
        providerSeparation + 3, userSeparation - providerSeparation - 3);
    std::string password = connectionUrl.substr(
        userSeparation + 1, passwordSeparation - userSeparation - 1);

    size_t hostSeparation = connectionUrl.find(":", passwordSeparation + 1);

    if (hostSeparation == std::string::npos) {
      std::cerr << "\033[1;31m[-] Database host not found\033[0m" << std::endl;
      return;
    }

    size_t portSeparation = connectionUrl.find("/", hostSeparation + 1);

    if (portSeparation == std::string::npos) {
      std::cerr << "\033[1;31m[-] Database port not found\033[0m" << std::endl;
      return;
    }

    std::string host = connectionUrl.substr(
        passwordSeparation + 1, hostSeparation - passwordSeparation - 1);
    std::string port = connectionUrl.substr(
        hostSeparation + 1, portSeparation - hostSeparation - 1);

    if (connectionUrl.size() - 1 <= portSeparation) {
      std::cerr << "\033[1;31m[-] Database name not found\033[0m" << std::endl;
      return;
    }

    std::string database = connectionUrl.substr(portSeparation + 1);

    db = Database::CreateInstance(database, user, password, host, port, false);
  }
}

void populateRoutes(std::vector<std::string> headers) {
  std::string routesValue = "";
  std::string headersValue = "";

  for (auto header : headers) {
    headersValue += "#include \"" +
                    config["endpoint_folder"].get<std::string>() + "/" +
                    header + "\"\n";

    std::string path = config["server_location"].get<std::string>() + "/" +
                       config["endpoint_folder"].get<std::string>() + "/" +
                       header;
    
    std::ifstream file(path);
    std::string line;

    if (!file.is_open()) {
      std::cerr << "Error opening file: " << path << std::endl;

      if (file.bad()) {
        std::cerr << "Fatal error: badbit is set." << std::endl;
      }

      if (file.fail()) {
        std::cerr << "Error details: " << strerror(errno) << std::endl;

#if defined(_WIN32)
        if (errno == EACCES) {
          HANDLE hFile =
              CreateFileA(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, NULL);

          if (hFile == INVALID_HANDLE_VALUE) {
            DWORD dw = GetLastError();
            if (dw == ERROR_SHARING_VIOLATION) {
              std::cout << "Requested file is locked. Waiting for file to be "
                           "unlocked..."
                        << std::endl;

              auto start = std::chrono::high_resolution_clock::now();
              while (true) {
                hFile = CreateFileA(path.c_str(), GENERIC_READ, 0, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile != INVALID_HANDLE_VALUE) {
                  CloseHandle(hFile);
                  Sleep(100);
                  file.open(path);
                  if (file.is_open()) {
                    std::cout << "File is unlocked. Continuing..." << std::endl;
                  } else {
                    std::cerr << "Error opening file: " << path << std::endl;
                    break;
                  }
                  break;
                }

                auto end = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::seconds>(end -
                                                                     start)
                        .count();

                if (duration >= 5) {
                  std::cerr << "File is still locked. Exiting..." << std::endl;
                  return;
                }
              }
            }
          }
        }
      }
#endif
    }

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

        size_t funcNameIndex = line.find(",", pathEndIndex);
        size_t funcNameEndIndex = line.find(")", funcNameIndex);

        std::string funcName = line.substr(
            funcNameIndex + 2, funcNameEndIndex - funcNameIndex - 2);

        std::string methodCopy = method;
        std::transform(method.begin(), method.end(), method.begin(), ::tolower);

        routesValue += "\trouter.AddRoute(\"" + methodCopy + "\", \"" + route +
                       "\", " + method + "_" + funcName + ");\n";
      }
    }
    file.close();
  }

  std::string templateCopy = cppTemplate;
  templateCopy.replace(templateCopy.find("{{ROUTES}}"), 10, routesValue);
  templateCopy.replace(templateCopy.find("{{HEADERS}}"), 11, headersValue);

  std::ofstream fileTemplate(config["server_location"].get<std::string>() +
                                 "/server.cpp",
                             std::ios::trunc);
  fileTemplate << templateCopy;

  fileTemplate.close();
}
}

#if defined(__linux__) || defined(__APPLE__)
void *loadLibrary(const char *libPath) {

  void *libHandle = dlmopen(LM_ID_NEWLM, libPath, RTLD_NOW | RTLD_LOCAL);
  if (!libHandle) {
    std::cerr << "Failed to load library: " << dlerror() << std::endl;
    return nullptr;
  }
  return libHandle;
}
#elif defined(_WIN32)
HINSTANCE loadLibrary(const char *libPath) {
  HMODULE libHandle = LoadLibrary(libPath);
  if (!libHandle) {
    std::cerr << "Failed to load library: " << GetLastError() << std::endl;
    return nullptr;
  }
  return libHandle;
}
#endif

#if defined(__linux__) || defined(__APPLE__)
void watchFiles() {
  loadConfig();

  int fd = inotify_init();
  if (fd < 0) {
    std::cerr << "Failed to initialize inotify" << std::endl;
    return;
  }

  int wd =
      inotify_add_watch(fd,
                        (config["server_location"].get<std::string>() + "/" +
                         config["endpoint_folder"].get<std::string>())
                            .data(),
                        IN_CREATE | IN_DELETE | IN_MODIFY);
  if (wd < 0) {
    std::cerr << "Failed to add watch" << std::endl;
    return;
  }

  char buffer[4096];
  std::vector<std::string> headers;

  DIR *dir;
  dirent *ent;
  if ((dir = opendir((config["server_location"].get<std::string>() + "/" +
                      config["endpoint_folder"].get<std::string>())
                         .data())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_REG &&
          ((std::string)ent->d_name).find(".hpp") != std::string::npos) {
        headers.push_back(ent->d_name);
      }
    }
    closedir(dir);
  }

  bool shouldCompile = false;
  bool shouldReload = false;

  populateRoutes(headers);
  instantiateDB();

  system(std::string("cmake -S " +
                     config["server_location"].get<std::string>() + " -B " +
                     config["server_location"].get<std::string>() + "/out")
             .c_str());
  system(std::string("cmake --build " +
                     config["server_location"].get<std::string>() + "/out")
             .c_str());

  void *serverLib = loadLibrary(
      std::string(config["server_location"].get<std::string>() + "/out/libserver.so")
          .c_str());

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
      system(
          std::string("rm -rf " + config["server_location"].get<std::string>() + "/out")
              .c_str());
      system(std::string("cmake -S " +
                         config["server_location"].get<std::string>() + " -B " +
                         config["server_location"].get<std::string>() + "/out")
                 .c_str());
      system(std::string("cmake --build " +
                         config["server_location"].get<std::string>() + "/out")
                 .c_str());

      serverLib =
          loadLibrary(std::string(config["server_location"].get<std::string>() +
                                  "/out/libserver.so")
                          .c_str());

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
#elif defined(_WIN32)
void watchFiles() {
  loadConfig();

  std::string serverLocation = config["server_location"].get<std::string>();
  std::string endpointFolder = config["endpoint_folder"].get<std::string>();
  std::string buildLocation =
      config["server_location"].get<std::string>() + "/out";

  std::string serverPath = serverLocation + "/server.cpp";
  std::string buildPath = buildLocation + "/libserver.dll";

  std::vector<std::string> headers;

  WIN32_FIND_DATAA findFileData;
  HANDLE hFind = FindFirstFileA(
      (serverLocation + "/" + endpointFolder + "/*").c_str(), &findFileData);
  if (hFind == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to find files in endpoint folder" << std::endl;
    return;
  }

  do {
    if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      continue;
    }

    std::string fileName = findFileData.cFileName;
    if (fileName.find(".hpp") != std::string::npos) {
      headers.push_back(fileName);
    }
  } while (FindNextFileA(hFind, &findFileData) != 0);

  FindClose(hFind);

  bool shouldCompile = false;
  bool shouldReload = false;

  populateRoutes(headers);
  system(std::string("cmake -S " + serverLocation + " -B " + buildLocation)
             .c_str());
  system(std::string("cmake --build " + buildLocation).c_str());

  auto start = std::chrono::high_resolution_clock::now();
  while (!std::filesystem::exists(buildPath)) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (duration >= 10) {
      std::cerr << "Failed to compile server" << std::endl;
      return;
    }
  }

  HINSTANCE serverLib = loadLibrary(buildPath.c_str());

  if (!serverLib) {
    return;
  }

  instantiateRoutes =
      (instantiateRoutesFunc)GetProcAddress(serverLib, "instantiateRoutes");

  if (!instantiateRoutes) {
    std::cerr << "Failed to load instantiateRoutes: " << GetLastError()
              << std::endl;
    return;
  }

  std::jthread serverThread(server, std::ref(shouldReload),
                            std::ref(reloadMutex));

  HANDLE hDir = CreateFileA(
      (serverLocation + "/" + endpointFolder).c_str(), FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
  if (hDir == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open directory: " << GetLastError() << std::endl;
  }

  FILE_NOTIFY_INFORMATION notifyInfo[1024];
  DWORD bytesReturned = 0;
  while (true) {
    auto res = ReadDirectoryChangesW(hDir, &notifyInfo, sizeof(notifyInfo),
                                     TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE,
                                     &bytesReturned, NULL, NULL);

    if (!res) {
      std::cerr << "Failed to read directory changes: " << GetLastError()
                << std::endl;
      return;
    }

    for (DWORD i = 0; i < bytesReturned;) {
      FILE_NOTIFY_INFORMATION *info =
          (FILE_NOTIFY_INFORMATION *)((char *)notifyInfo + i);
      std::wstring fileName(info->FileName, info->FileNameLength / 2);

      if (info->Action == FILE_ACTION_MODIFIED) {
        shouldCompile = true;
      } else if (info->Action == FILE_ACTION_ADDED) {
        std::string fileNameStr(fileName.begin(), fileName.end());
        if (fileNameStr.find(".hpp") != std::string::npos &&
            std::find(headers.begin(), headers.end(), fileNameStr) ==
                headers.end()) {
          headers.push_back(fileNameStr);
        }
      } else if (info->Action == FILE_ACTION_REMOVED) {
        std::string fileNameStr(fileName.begin(), fileName.end());
        if (fileNameStr.find(".hpp") != std::string::npos) {
          std::erase(headers, fileNameStr);
        }
      }

      if (info->NextEntryOffset == 0) {
        break;
      }

      i += info->NextEntryOffset;
    }

    if (shouldCompile) {
      std::cout << "\033[1;34m[*] Compiling...\033[0m" << std::endl;

      if (serverLib) {
        FreeLibrary(serverLib);
        serverLib = nullptr;
        instantiateRoutes = nullptr;
      }

      for (auto header : headers) {
        std::cout << header << std::endl;
      }

      populateRoutes(headers);
      system(std::string("cmake -S " + serverLocation + " -B " + buildLocation)
                 .c_str());
      system(std::string("cmake --build " + buildLocation).c_str());

      serverLib = loadLibrary(buildPath.c_str());

      if (!serverLib) {
        continue;
      }

      instantiateRoutes =
          (instantiateRoutesFunc)GetProcAddress(serverLib, "instantiateRoutes");

      if (!instantiateRoutes) {
        std::cerr << "Failed to load instantiateRoutes: " << GetLastError()
                  << std::endl;
        continue;
      }

      shouldCompile = false;
      {
        std::lock_guard<std::mutex> lock(reloadMutex);
        shouldReload = true;
      }
    }
  }

  CloseHandle(hDir);

  FreeLibrary(serverLib);

  return;
}
#endif
