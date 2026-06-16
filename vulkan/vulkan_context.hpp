#pragma once
#include "gfx_iface.hpp"
#include <vector>
#include <volk.h>

namespace aeo {
namespace vulkan {
class context final : public aeo::gfx::context {
public:
  context(const aeo::gfx::config &config, void *window, void *display,
          VkSurfaceKHR surface = VK_NULL_HANDLE);
  ~context() override;
  u32 query_for_device(aeo::gfx::device_type preferred_type,
                       aeo::gfx::hardware_desc *device, u32 count) override;
  result_t init_context(aeo::gfx::hardware_desc &device) override;
  result_t init_swapchain(u32 width, u32 height) override;

  aeo::gfx::config get_config() { return config; }
  VkInstance get_instance() { return instance; }
  VkDevice get_device() { return device; }
  VkPhysicalDevice get_hardware() { return hardware; }

private:
  result_t create_instance();
  result_t create_headless_surface();
  result_t create_surface(void *display, void *window);
  bool has_extension(const char *ext, std::vector<VkExtensionProperties> &prop);

private:
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice hardware = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugger = VK_NULL_HANDLE;
  aeo::gfx::config config;
  u32 validation_layer_version;

  bool has_EXT_swapchain_colorspace = false;
  std::vector<const char *> enabled_instance_extensions;
  std::vector<const char *> enabled_device_extensions;
};
} // namespace vulkan
} // namespace aeo
