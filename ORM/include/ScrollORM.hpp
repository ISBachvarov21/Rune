#pragma once

#include "../../dependencies/CppHttp/include/nlohmann/json.hpp"
#include "../../dependencies/soci/include/soci/soci.h"
#include "../../dependencies/soci/include/soci/postgresql/soci-postgresql.h"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#define EXPORT extern "C" __attribute__((visibility("default")))
#elif defined(_WIN32)
#include <windows.h>
#include <filesystem>
#define EXPORT extern "C" __declspec(dllexport)
#endif

using json = nlohmann::json;

static std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> tables;

EXPORT void reflectModels(json config);
EXPORT void migrateDB(json config, std::string migationName); 
