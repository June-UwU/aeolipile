#include "vulkan_context.hpp"
#include "define.hpp"
#include "log.hpp"
#include <cstring>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(__linux__)
#if defined(AEO_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif
#define GLFW_EXPOSE_NATIVE_X11
#define VK_USE_PLATFORM_XLIB_KHR

#endif
#elif
#error "too lazy to implement this now :)"
#endif

#include "vk_defines.hpp"
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

namespace aeo {

namespace gfx {

std::unique_ptr<aeo::gfx::context> create_context_with_swapchain(
    GLFWwindow *window, u32 width, u32 height, const aeo::gfx::config &config,
    aeo::gfx::device_type preferred_device, s32 selected_device) {
  std::unique_ptr<aeo::vulkan::context> ctx;

#if defined(__linux__)
#if defined(AEO_WAYLAND)
  wl_surface *wayland_window = glfwGetWaylandWindow(window);
  if (!config.enable_headless && !wayland_window) {
    AEO_ASSERT(false, "no assioated window, are sure to run set the right "
                      "config? config::enable_headless and window\n");
    return nullptr;
  }
  ctx = std::make_unique<aeo::vulkan::context>(config, wayland_window,
                                               (void *)glfwGetWaylandDisplay());
#else
  ctx = std::make_unique<aeo::vulkan::context>(
      config, (void *)glfwGetX11Window(window), (void *)glfwGetX11Display());
#endif
#else
#error "too lazy to support every other platform at the moment"
#endif

  return std::move(ctx);
}

} // namespace gfx

namespace vulkan {

VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *data, void *user_data) {
  if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    return VK_FALSE;
  }

  const bool is_error =
      (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0;
  const bool is_warning =
      (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0;

  const size_t len = data->pMessage ? strlen(data->pMessage) : 128u;

  AEO_ASSERT(len < 65536, "message is too long");

  char *error_name = (char *)alloca(len + 1);
  int object = 0;
  void *handle = nullptr;
  char type_name[128] = {};
  void *message_id = nullptr;

  context *ctx = static_cast<context *>(user_data);

  if (!is_error && !is_warning && data->pMessageIdName) {
    if (strcmp(data->pMessageIdName, "Loader Message") == 0) {
      return VK_FALSE;
    }
  }

  if (sscanf(data->pMessage,
             "Validation Error: [ %[^]] ] Object %i: handle = %p, type = %127s "
             "| MessageID = %p",
             error_name, &object, &handle, type_name, &message_id) >= 2) {
    const char *message = strrchr(data->pMessage, '|') + 1;

    if (ctx->get_config().crash_on_validation_error) {
      TLOGF("%s%s \n\tObject: %d \n\thandle: %p \n\ttype: %s \n\t message id: "
            "%p \n\t %s\n",
            is_error ? "Validation Error: " : "Validation Warning: ",
            error_name, object, handle, type_name, message_id, message);
    } else {
      TLOGW("%s%s \n\tObject: %d \n\thandle: %p \n\ttype: %s \n\t message id: "
            "%p \n\t %s\n",
            is_error ? "Validation Error: " : "Validation Warning: ",
            error_name, object, handle, type_name, message_id, message);
    }

  } else {
    if (ctx->get_config().crash_on_validation_error) {
      TLOGW("%s%s", is_error ? "Validation Error: " : "Validation Warning: ",
            data->pMessage);
    } else {
      TLOGW("%s%s", is_error ? "Validation Error: " : "Validation Warning: ",
            data->pMessage);
    }
  }

  if (is_error) {
    if (ctx->get_config().crash_on_validation_error) {
      AEO_ASSERT(false, "validation error crash, refer earlier logs!");
      std::terminate();
    }
  }

  return VK_FALSE;
}

const char *DEFAULT_VALIDATION_LAYERS[] = {"VK_LAYER_KHRONOS_validation"};

context::context(const aeo::gfx::config &config, void *window, void *display,
                 VkSurfaceKHR surface)
    : surface(surface), config(config) {
  AEO_ASSERT(VK_SUCCESS == volkInitialize(), "failed to initialize volk\n");
  result_t result = create_instance();
  AEO_ASSERT(false == is_error(result), "failed to create vulkan instance\n");

  if (VK_NULL_HANDLE == surface) {
    if (true == config.enable_headless) {
      TLOGL("running headless\n");
      result = create_headless_surface();
    } else {
      TLOGL("running with window context\n");
      result = create_surface(display, window);
    }
  }
  AEO_ASSERT(false == is_error(result), "failed to create vulkan surface\n");
}

context::~context() {}

bool context::has_extension(const char *ext,
                            std::vector<VkExtensionProperties> &props) {
  for (const VkExtensionProperties &p : props) {
    if (0 == strcmp(ext, p.extensionName)) {
      return true;
    }
  }
  return false;
}

result_t context::create_instance() {
  instance = VK_NULL_HANDLE;

  {
    u32 num_layer_properties = 0;
    vkEnumerateInstanceLayerProperties(&num_layer_properties, nullptr);
    std::vector<VkLayerProperties> layer_properties(num_layer_properties);
    vkEnumerateInstanceLayerProperties(&num_layer_properties,
                                       layer_properties.data());

    [this, &layer_properties]() -> void {
      for (const VkLayerProperties &props : layer_properties) {
        for (const char *layer : DEFAULT_VALIDATION_LAYERS) {
          if (!strcmp(props.layerName, layer)) {
            validation_layer_version = props.specVersion;
            return;
          }
        }
      }
      if (true == config.enable_validation) {
        TLOGP("disabling logging due to inavailability..!!\n");
      }
      config.enable_validation = false;
    }();
  }

  std::vector<VkExtensionProperties> all_instance_extensions;
  {
    u32 count = 0;
    VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr),
              "failed to enumerate instance extension properties\n");
    all_instance_extensions.resize(count);
    VK_ASSERT(vkEnumerateInstanceExtensionProperties(
                  nullptr, &count, all_instance_extensions.data()),
              "failed to enumerate instance extension properties\n");
  }

  if (true == config.enable_validation) {
    for (const char *layer : DEFAULT_VALIDATION_LAYERS) {
      u32 count = 0;
      VK_ASSERT(vkEnumerateInstanceExtensionProperties(layer, &count, nullptr),
                "failed to enumerate instance extension properties\n");

      if (count > 0) {
        const size_t size = all_instance_extensions.size();
        all_instance_extensions.resize(size + count);
        VK_ASSERT(vkEnumerateInstanceExtensionProperties(
                      layer, &count, all_instance_extensions.data() + size),
                  "failed to get validation layer\n");
      }
    }
  }

  enabled_instance_extensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(__linux__)
#if defined(AEO_WAYLAND)
      VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#else
      VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#elif
#error "too lazy to implement other platforms"
#endif
  };

  const bool has_portability_extension = has_extension(
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, all_instance_extensions);
  if (true == has_portability_extension) {
    enabled_instance_extensions.push_back(
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  }

  const bool has_debug_utils =
      has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, all_instance_extensions);
  if (true == has_debug_utils) {
    enabled_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  if (config.enable_validation) {
    enabled_instance_extensions.push_back(
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME); // enabled only for
                                                    // validation
  }

  if (config.enable_headless) {
    enabled_instance_extensions.push_back(
        VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
  }

  if (has_extension(VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
                    all_instance_extensions)) {
    enabled_instance_extensions.push_back(
        VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
  }
  // Lavapipe 25.3.3 supports both `VK_KHR_surface_maintenance` and
  // `VK_EXT_surface_maintenance`, but only supports
  // `VK_EXT_swapchain_maintenance1`, which in turn requires
  // `VK_EXT_surface_maintenance1`
  if (has_extension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
                    all_instance_extensions)) {
    // remove once VK_KHR_surface_maintenance1 becomes mandatory
    enabled_instance_extensions.push_back(
        VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
  }

  if (has_extension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
                    all_instance_extensions)) {
    has_EXT_swapchain_colorspace = true;
    enabled_instance_extensions.push_back(
        VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
  }

  if (has_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
                    all_instance_extensions)) {
    // required by the instance extension VK_EXT_surface_maintenance1
    enabled_instance_extensions.push_back(
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
  }

  for (const char *ext : config.extension_instance) {
    if (ext) {
      enabled_instance_extensions.push_back(ext);
    }
  }

  bool missing_required_ext = false;
  for (const char *ext : enabled_instance_extensions) {
    if (false == has_extension(ext, all_instance_extensions)) {
      TLOGF("%s missing..!!!\n", ext);
      missing_required_ext = true;
    }
  }
  if (true == missing_required_ext) {
    return -E_FAIL;
  }

  const VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "Aeo/Vulkan",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Aeo/Vulkan",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = config.vk_version == aeo::gfx::vulkan_version::version_1_3
                        ? VK_API_VERSION_1_3
                        : VK_API_VERSION_1_4,
  };

  const VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = has_portability_extension
                   ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
                   : 0u,
      .enabledLayerCount = config.enable_validation
                               ? (u32)AEO_ARRAY_SIZE(DEFAULT_VALIDATION_LAYERS)
                               : 0,
      .ppEnabledLayerNames =
          config.enable_validation ? DEFAULT_VALIDATION_LAYERS : nullptr,
      .enabledExtensionCount = (u32)enabled_instance_extensions.size(),
      .ppEnabledExtensionNames = enabled_instance_extensions.data(),
  };

  VK_ASSERT(vkCreateInstance(&create_info, nullptr, &instance),
            "failed to create instance\n");
  volkLoadInstance(instance);

  if (true == has_debug_utils) {
    const VkDebugUtilsMessengerCreateInfoEXT debug_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkan_debug_callback,
        .pUserData = nullptr,
    };

    VK_ASSERT(vkCreateDebugUtilsMessengerEXT(instance, &debug_info, nullptr,
                                             &debugger),
              "failed to debugger\n");
  }

  TLOGL("%s version %d.%d.%d\n", DEFAULT_VALIDATION_LAYERS[0],
        VK_VERSION_MAJOR(validation_layer_version),
        VK_VERSION_MINOR(validation_layer_version),
        VK_VERSION_PATCH(validation_layer_version));

  TLOGL("Vulkan Available Extensions\n");
  for (const VkExtensionProperties &ext : all_instance_extensions) {
    LOGL("\t%s\n", ext.extensionName);
  }

  return SUCCESS;
}

result_t context::create_headless_surface() {
  const VkHeadlessSurfaceCreateInfoEXT create_info = {
      .sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
  };
  VK_ASSERT(
      vkCreateHeadlessSurfaceEXT(instance, &create_info, nullptr, &surface),
      "failed to create a headless surface\n");
  return SUCCESS;
}

result_t context::create_surface(void *display, void *window) {
#if defined(AEO_XLIB)
  const VkXlibSurfaceCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
      .flags = 0,
      .dpy = (Display *)display,
      .window = (Window)window,
  };
  auto vkCreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(
      vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR"));

  VK_ASSERT(vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &surface),
            "failed to create Xlib surface\n");
#elif defined(AEO_WAYLAND)
  const VkWaylandSurfaceCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
      .flags = 0,
      .display = (wl_display *)display,
      .surface = (wl_surface *)window,
  };
  auto vkCreateWaylandSurfaceKHR =
      reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(
          vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR"));

  VK_ASSERT(
      vkCreateWaylandSurfaceKHR(instance, &create_info, nullptr, &surface),
      "failed to create wayland surface\n");
#else
#error "implement your platform..!"
#endif
  return SUCCESS;
}

u32 context::query_for_device(aeo::gfx::device_type preferred_type,
                              aeo::gfx::hardware_desc *device, u32 count) {
  return 0;
}

result_t context::init_context(aeo::gfx::hardware_desc &device) {
  return SUCCESS;
}
result_t context::init_swapchain(u32 width, u32 height) { return SUCCESS; }
} // namespace vulkan
} // namespace aeo
