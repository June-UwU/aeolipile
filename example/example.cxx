#include "aeolipile.hpp"
#include "types.hpp"
#include <iostream>

int main() {
  aeo::result_t result = aeo::initialize_early_infrastructure();
  if (true == aeo::is_error(result)) {
    std::cout << aeo::result_to_string(result) << "\n";
    return -1;
  }

  GLFWwindow *main_window = aeo::init_window("example", 1080, 720);

  while (!glfwWindowShouldClose(main_window)) {
  }

  glfwDestroyWindow(main_window);
  aeo::deinitialize_early_infrastructure();
  return 0;
}
