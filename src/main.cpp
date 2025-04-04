#include "../include/Rune.hpp"

int main(int argc, char **argv) {
  std::iostream::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);

  if (argc < 2) {
    std::cout << "Options: \n"
              << " --init: Initialize a new Rune project in current directory\n"
              << " --run: Run Rune project" << std::endl;
    return 0;
  }

  if (std::string(argv[1]) == "--init") {
    // clone the Rune project template
    // repo url: https://github.com/ISBachvarov21/Rune-Template

    std::cout << "Cloning Rune project template..." << std::endl;

#if defined(_WIN32)
    std::string command = "git clone https://github.com/ISBachvarov21/Rune-Template . && rd .git /s /q";
#elif defined(__linux__) || defined(__APPLE__)
    std::string command = "git clone https://github.com/ISBachvarov21/Rune-Template . && rm -rf .git";
#endif
    system(command.c_str());
    
    std::cout << "Rune project initialized!" << std::endl;
  }
  else if (std::string(argv[1]) == "--run") {
    // run the Rune project
    std::cout << "Running Rune project..." << std::endl;
    watchFiles();
  }
  else if (std::string(argv[1]) == "--migrate") {
    // run the migrations
    std::string migrationName = "";

    if (argc == 3) {
      migrationName = argv[2];
    }

    json config = loadConfig();
    migrateDB(config, migrationName);
  }
  else {
    std::cout << "Invalid option!" << std::endl;
  }
}
