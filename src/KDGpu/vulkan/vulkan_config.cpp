/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_config.h"
#include <KDGpu/config.h>

#if defined(KDGPU_PLATFORM_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace KDGpu {

std::vector<const char *> getDefaultRequestedInstanceExtensions()
{
    std::vector<const char *> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(KDGPU_PLATFORM_LINUX)
    extensions.push_back("VK_KHR_xcb_surface");
    extensions.push_back("VK_KHR_wayland_surface");
#elif defined(KDGPU_PLATFORM_WIN32)
    extensions.push_back("VK_KHR_win32_surface");
#elif defined(KDGPU_PLATFORM_MACOS)
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
#if defined(KDGPU_PLATFORM_LINUX)
    extensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    extensions.push_back(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
#elif defined(KDGPU_PLATFORM_WIN32)
    extensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
    extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
    extensions.push_back(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
#endif
#if defined(VK_KHR_synchronization2)
    extensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
#endif
#if defined(KDGPU_PLATFORM_MACOS)
    extensions.push_back("VK_KHR_portability_subset");
#endif
    return extensions;
}

} // namespace KDGpu
