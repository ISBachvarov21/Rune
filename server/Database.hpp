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
    
    std::string connectionString = "dbname=" + databaseName + " user=" + username +
                                  " password=" + password + " host=" + host +
                                  " port=" + port;

    if (sslMode) {
      connectionString += " sslmode=require";
    }

    sql.open(soci::postgresql, connectionString);
    dbMutex.unlock();
  }

  static Database *databaseInstance;
  soci::session sql;

public:
  Database(const Database &) = delete;

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

      if (!config.contains("provider")) {
        std::cerr << "\033[1;31m[-] Database provider not found\033[0m"
                  << std::endl;
        return nullptr;
      }

      if (!config.contains("host")) {
        std::cerr << "\033[1;31m[-] Database host not found\033[0m" << std::endl;
        return nullptr;
      }

      if (!config.contains("port")) {
        std::cerr << "\033[1;31m[-] Database port not found\033[0m" << std::endl;
        return nullptr;
      }

      if (!config.contains("database")) {
        std::cerr << "\033[1;31m[-] Database name not found\033[0m" << std::endl;
        return nullptr;
      }

      if (!config.contains("user")) {
        std::cerr << "\033[1;31m[-] Database user not found\033[0m" << std::endl;
        return nullptr;
      }

      if (!config.contains("password")) {
        std::cerr << "\033[1;31m[-] Database password not found\033[0m"
                  << std::endl;
        return nullptr;
      }

      std::string provider = config["provider"].get<std::string>();
      std::string host = config["host"].get<std::string>();
      std::string port = std::to_string(config["port"].get<int>());
      std::string database = config["database"].get<std::string>();
      std::string user = config["user"].get<std::string>();
      std::string password = config["password"].get<std::string>();

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
