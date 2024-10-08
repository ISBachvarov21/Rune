/*
 * TODO: need to compile server files when files are modified
 *       need to reload server dll when files are modified
 *
 *       need to add server/routes header parsing
*/
#include "../include/Rune.hpp"

int main(int argc, char **argv) {
  std::iostream::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);

  watchFiles();
}
