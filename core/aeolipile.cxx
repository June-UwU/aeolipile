#include "aeolipile.hpp"
#include "log.hpp"

namespace aeo {
constexpr u32 version = medusa::make_version(0, 0, 1);
void error_callback(int error, const char *description) {
  TLOGF("glfw failed with %d, \n%s", error, description);
}

result_t initialize_early_infrastructure() {
  aeo::logging::initialize();
  TLOGL("aeolipile (vulkan renderer library) version %d.%d.%d\n",
        medusa::get_major(version), medusa::get_minor(version),
        medusa::get_patch(version));

  if (!glfwInit()) {
    TLOGF("%s\n", "failed to initialize glfw!");
    return -E_FAIL;
  }
  glfwSetErrorCallback(error_callback);

  return SUCCESS;
}

result_t deinitialize_early_infrastructure() {
  glfwTerminate();
  return SUCCESS;
}

GLFWwindow *init_window(const std::string title, const u32 width,
                        const u32 height) {
  GLFWwindow *window =
      glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

  if (nullptr == window) {
    TLOGF("failed to create a window (%s)\n", title.c_str());
  }

  return window;
}

u32 get_version() { return version; }

}; // namespace aeo
