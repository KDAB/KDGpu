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

#if defined(KDGPU_PLATFORM_LINUX)
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;
typedef uint32_t xcb_visualid_t;

#include <vulkan/vulkan_xcb.h>

struct wl_display;
struct wl_surface;
#include <vulkan/vulkan_wayland.h>
#endif

#if defined(KDGPU_PLATFORM_APPLE)
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_beta.h>
#endif

#if defined(KDGPU_PLATFORM_ANDROID)
#include <vulkan/vulkan_android.h>
#endif

namespace KDGpu {

std::vector<const char *> getDefaultRequestedInstanceExtensions()
{
    std::vector<const char *> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_KHR_xcb_surface)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_KHR_wayland_surface)
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_KHR_win32_surface)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(KDGPU_PLATFORM_APPLE)
        VK_EXT_METAL_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
#elif defined(KDGPU_PLATFORM_ANDROID)
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
    };

#if defined(VK_EXT_debug_utils)
    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    return extensions;
}

//
// Device Config
//
std::vector<const char *> getDefaultRequestedDeviceExtensions()
{
    std::vector<const char *> extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined(VK_EXT_host_image_copy)
        VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME, // Needed by VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME
        VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME, // Needed by VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME
        VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
#endif
#if defined(VK_KHR_external_semaphore_fd)
        VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
#endif
#if defined(VK_KHR_external_semaphore_win32)
        VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#endif
#if defined(VK_KHR_external_fence_fd)
        VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,
#endif
#if defined(VK_KHR_external_fence_win32)
        VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
#endif
#if defined(VK_KHR_external_memory_fd)
        VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
#endif
#if defined(VK_KHR_external_memory_win32)
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
#endif
#if defined(VK_EXT_external_memory_dma_buf)
        VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
#endif
#if defined(VK_KHR_deferred_host_operations)
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
#endif
#if defined(VK_KHR_ray_tracing_pipeline)
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
#endif
#if defined(VK_KHR_acceleration_structure)
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
#endif
#if defined(VK_EXT_mesh_shader)
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
#endif
#if defined(VK_EXT_image_drm_format_modifier)
        VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
#endif
#if defined(VK_KHR_synchronization2)
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
#endif
#if defined(KDGPU_PLATFORM_MACOS)
        VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
#if defined(VK_KHR_shader_non_semantic_info)
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
#endif
#if defined(VK_KHR_sampler_ycbcr_conversion)
        VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
#endif
#if defined(VK_KHR_push_descriptor)
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
#endif
#if defined(VK_KHR_dynamic_rendering)
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
#endif
#if defined(VK_KHR_dynamic_rendering_local_read)
        VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME
#endif
    };

    return extensions;
}

} // namespace KDGpu
