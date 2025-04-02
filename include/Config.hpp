#pragma once

#include "../dependencies/CppHttp/include/nlohmann/json.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

json loadConfig();
