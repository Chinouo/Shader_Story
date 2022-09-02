#include <cassert>
#include <iostream>

#include "engine/engine.hpp"

int main(int, char**) {
  try {
    using namespace ShaderStory;
    Engine e;
    e.InitializeEngine();
    // e.DebugSingleThread();
    e.RunInitialedEngine();
    e.ShutDowmEngine();
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
