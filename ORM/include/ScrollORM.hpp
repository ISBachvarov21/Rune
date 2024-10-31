#pragma once

#include "../../dependencies/CppHttp/include/nlohmann/json.hpp"
#include <string>
#include <fstream>
#include <iostream>

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

EXPORT void reflectModels(json config); 
