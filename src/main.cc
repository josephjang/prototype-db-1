#include <iostream>
#include "spdlog/spdlog.h"

auto sum(int a, int b) { return a + b; }

int main() {
  std::cout << "Hello, world!" << std::endl;
  spdlog::info("Hello, spdlog!");

  return 0;
}