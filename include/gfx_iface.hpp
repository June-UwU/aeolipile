#pragma once
#include "handle.hpp"
#include "types.hpp"
#include <GLFW/glfw3.h>
#include <memory>

namespace aeo {
namespace gfx {

enum class vulkan_version {
  version_1_3 = 0x0,
  version_1_4,
};

enum class color_space {
  srgb_nonlinear = 0x0,
  srgb_extended_linear,
  hdr10,
  bt709_linear,
};

enum class present_mode {
  fifo = 0x0,
  fifo_relaxed,
  immediate,
  mailbox,
  shared_demand_refresh,
  shared_continuous_refresh,
  fifo_latest_ready,
};

enum class device_type {
  integrated = 0x0,
  external,
  discrete,
  software,
};

struct hardware_desc {
  enum { DEVICE_NAME_SIZE = 256 };
  uintptr_t guid = 0;
  device_type type = device_type::software;
  char name[DEVICE_NAME_SIZE] = {0};
};

constexpr const u32 MAX_PRESENT_MODES = 8;
constexpr const u32 MAX_CUSTOM_EXTENSIONS = 32;

struct config {
  vulkan_version vk_version = vulkan_version::version_1_3;
  bool crash_on_validation_error = false;
  bool enable_validation = true;
  color_space requested_swapchain_color_space =
      color_space::srgb_extended_linear;
  present_mode present_modes[MAX_PRESENT_MODES] = {
#if defined(__linux__) || defined(_M_ARM64)
      present_mode::immediate,
#endif
      present_mode::mailbox, present_mode::fifo_relaxed, present_mode::fifo};
  const char *extension_instance[MAX_CUSTOM_EXTENSIONS]{};
  const char *extension_device[MAX_CUSTOM_EXTENSIONS]{};
  void *extensions_device_features = nullptr;
  bool enable_headless = false;
  u64 max_staging_buffer_size = 128 * 1024 * 1024;
};

class context;
class command_buffer;

std::unique_ptr<aeo::gfx::context> create_context_with_swapchain(
    GLFWwindow *window, u32 width, u32 height, const aeo::gfx::config &config,
    aeo::gfx::device_type preferred_device = device_type::discrete,
    s32 selected_device = -1);

class context {
protected:
  context() = default;

public:
  virtual ~context() = default;
  virtual u32 query_for_device(device_type preferred_type,
                               hardware_desc *device, u32 count) = 0;
  virtual result_t init_context(hardware_desc &device) = 0;
  virtual result_t init_swapchain(u32 width, u32 height) = 0;
};

class command_buffer {
public:
  virtual ~command_buffer() = default;
};
} // namespace gfx
} // namespace aeo
