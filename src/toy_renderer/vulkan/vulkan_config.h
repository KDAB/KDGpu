#pragma once

#include <vulkan/vulkan.h>

#include <array>

namespace ToyRenderer {

//
// Instance Config
//

#if defined(NDEBUG) || defined(__arm__)
constexpr bool enableValidationLayers = false;
const std::vector<const char *> requestedInstanceLayers = {};
#else
constexpr bool enableValidationLayers = true;
const std::vector<const char *> requestedInstanceLayers = {
    "VK_LAYER_KHRONOS_validation",
};
#endif

constexpr std::array<const char *, 1> requestedInstanceExtensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

std::vector<const char *> getDefaultRequestedInstanceExtensions()
{
    std::vector<const char *> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(PLATFORM_LINUX)
    extensions.push_back("VK_KHR_xcb_surface");
#if defined(PLATFORM_WAYLAND)
    extensions.push_back("VK_KHR_wayland_surface");
#endif
#elif defined(PLATFORM_WIN32)
    extensions.push_back("VK_KHR_win32_surface");
#elif defined(PLATFORM_MACOS)
    extensions.push_back("VK_EXT_metal_surface");
#endif
    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

//
// Device Config
//
std::vector<const char *> getDefaultRequestedDeviceExtensions()
{
    std::vector<const char *> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#if defined(PLATFORM_MACOS)
    extensions.push_back("VK_KHR_portability_subset");
#endif
    return extensions;
}

// This determines the maximum number of frames that can be in-flight at any one time.
// With the default setting of 2, we can be recording the commands for frame N+1 whilst
// the GPU is executing those for frame N. We cannot then record commands for frame N+2
// until the GPU signals it is done with frame N.
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

} // namespace ToyRenderer
