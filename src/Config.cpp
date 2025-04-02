#include "../include/Config.hpp"

json loadConfig() {
  json config;
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
    return config;
  }
  if (!config.contains("port") || !config["port"].is_number_integer()) {
    std::cerr << "Invalid port" << std::endl;
    return config;
  }
  if (!config.contains("host") || !config["host"].is_string()) {
    std::cerr << "Invalid host" << std::endl;
    return config;
  }
  if (!config.contains("server_location") ||
      !config["server_location"].is_string()) {
    std::cerr << "Invalid server location" << std::endl;
    return config;
  }
  if (!config.contains("endpoint_folder") ||
      !config["endpoint_folder"].is_string()) {
    std::cerr << "Invalid endpoint folder" << std::endl;
    return config;
  }

  file.close();

  return config;
}
