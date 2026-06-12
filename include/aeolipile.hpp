#pragma once
#include "types.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace aeo {
result_t initialize_early_infrastructure();
result_t deinitialize_early_infrastructure();
GLFWwindow *init_window(const std::string title, const u32 width,
                        const u32 height);
u32 get_version();
}; // namespace aeo
