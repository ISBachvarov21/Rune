#pragma once

#include "../dependencies/soci/include/soci/soci.h"
#include "soci/postgresql/soci-postgresql.h"
#include <mutex>
#include <string>

using namespace soci;

class Database {
private:
  Database(std::string databaseName, std::string username, std::string password,
           std::string host, std::string port, bool sslMode = true) {
    dbMutex.lock();

    std::string sslModeStr =
        sslMode ? (std::string) " sslmode=require" : (std::string) "";
    this->sql.open(postgresql, "dbname=" + databaseName + " user=" + username +
                                   " password=" + password + " host=" + host +
                                   " port=" + port + sslModeStr);
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
      databaseInstance =
          new Database(databaseName, username, password, host, port, sslMode);
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
