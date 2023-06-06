/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <vulkan/vulkan.h>

#include <array>

namespace KDGpu {

//
// Instance Config
//

#if defined(NDEBUG) || defined(__arm__)
constexpr bool enableValidationLayers = false;
const std::vector<const char *> requestedInstanceLayers = {
#if defined(PLATFORM_MACOS)
    "VK_LAYER_KHRONOS_synchronization2"
#endif
};
#else
constexpr bool enableValidationLayers = true;
const std::vector<const char *> requestedInstanceLayers = {
    "VK_LAYER_KHRONOS_validation",
#if defined(PLATFORM_MACOS)
    "VK_LAYER_KHRONOS_synchronization2"
#endif
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
    extensions.push_back("VK_KHR_wayland_surface");
#elif defined(PLATFORM_WIN32)
    extensions.push_back("VK_KHR_win32_surface");
#elif defined(PLATFORM_MACOS)
    extensions.push_back("VK_EXT_metal_surface");
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
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
    extensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
#if defined(PLATFORM_MACOS)
    extensions.push_back("VK_KHR_portability_subset");
#endif
    return extensions;
}

} // namespace KDGpu
