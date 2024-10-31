#pragma once

#include "../dependencies/soci/include/soci/soci.h"
#include "../dependencies/soci/include/soci/postgresql/soci-postgresql.h"
#include <mutex>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../dependencies/CppHttp/include/nlohmann/json.hpp"

using json = nlohmann::json;

struct alignas(64) DatabaseInfo {
  char dbConfig[512];
};

class Database {
private:
  Database(std::string databaseName, std::string username, std::string password,
           std::string host, std::string port, bool sslMode = true) {
    dbMutex.lock();
    sql.open(soci::postgresql, "dbname=postgres user=postgres password=postgres host=localhost port=5432");
    dbMutex.unlock();
  }

  static Database *databaseInstance;
  soci::session sql;

public:
  Database(const Database &) = delete;

  static Database *CreateInstance(std::string databaseName,
                                  std::string username, std::string password,
                                  std::string host, std::string port,
                                  bool sslMode = true) {
    if (databaseInstance == nullptr) {
    }
    return databaseInstance;
  }

  static Database *GetInstance() {
    if (databaseInstance == nullptr) {
      int fd = shm_open("/db_connection_details", O_RDWR, 0666);
      DatabaseInfo* dbState = (DatabaseInfo*) mmap(nullptr, 
          sizeof(DatabaseInfo),
          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0
      );

      DatabaseInfo dbInfo = *dbState;

      close(fd);

      json config = json::parse(dbInfo.dbConfig);

      if (!config.contains("connection_url")) {
        std::cerr << "\033[1;31m[-] Database connection url not found\033[0m"
                  << std::endl;
      }
      std::string connectionUrl =
          config["connection_url"].get<std::string>();

      size_t providerSeparation = connectionUrl.find("://");

      if (providerSeparation == std::string::npos) {
        std::cerr << "\033[1;31m[-] Database provider not found\033[0m"
                  << std::endl;
        return nullptr;
      }

      std::string provider = connectionUrl.substr(0, providerSeparation);

      if (provider != "postgresql") {
        std::cerr << "\033[1;31m[-] Database provider not supported\033[0m"
                  << std::endl;
        return nullptr;
      }

      size_t userSeparation = connectionUrl.find(":", providerSeparation + 3);

      if (userSeparation == std::string::npos) {
        std::cerr << "\033[1;31m[-] Database user not found\033[0m" << std::endl;
        return nullptr;
      }

      size_t passwordSeparation = connectionUrl.find("@", userSeparation + 1);

      if (passwordSeparation == std::string::npos) {
        std::cerr << "\033[1;31m[-] Database password not found\033[0m"
                  << std::endl;
        return nullptr;
      }

      std::string user = connectionUrl.substr(
          providerSeparation + 3, userSeparation - providerSeparation - 3);
      std::string password = connectionUrl.substr(
          userSeparation + 1, passwordSeparation - userSeparation - 1);

      size_t hostSeparation = connectionUrl.find(":", passwordSeparation + 1);

      if (hostSeparation == std::string::npos) {
        std::cerr << "\033[1;31m[-] Database host not found\033[0m" << std::endl;
        return nullptr;
      }

      size_t portSeparation = connectionUrl.find("/", hostSeparation + 1);

      if (portSeparation == std::string::npos) {
        std::cerr << "\033[1;31m[-] Database port not found\033[0m" << std::endl;
        return nullptr;
      }

      std::string host = connectionUrl.substr(
          passwordSeparation + 1, hostSeparation - passwordSeparation - 1);
      std::string port = connectionUrl.substr(
          hostSeparation + 1, portSeparation - hostSeparation - 1);

      if (connectionUrl.size() - 1 <= portSeparation) {
        std::cerr << "\033[1;31m[-] Database name not found\033[0m" << std::endl;
        return nullptr;
      }

      std::string database = connectionUrl.substr(portSeparation + 1);

      databaseInstance =
          new Database(database, user, password, host, port, false);
    }
    return databaseInstance;
  }

  soci::session *GetSession() { return &sql; }

  void Close() {
    sql.close();
    delete databaseInstance;
    databaseInstance = nullptr;
  }

  static std::mutex dbMutex;
};
