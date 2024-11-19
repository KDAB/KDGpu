/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/gpu_semaphore.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/texture.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanSwapchain
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanSwapchain {
    explicit VulkanSwapchain(VkSwapchainKHR _swapchain,
                             Format _format,
                             Extent3D _extent,
                             uint32_t _arrayLayers,
                             TextureUsageFlags _imageUsageFlags,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle);

    std::vector<Handle<Texture_t>> getTextures();

    AcquireImageResult getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore);

    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    Format format;
    Extent3D extent;
    uint32_t arrayLayers;
    TextureUsageFlags imageUsageFlags;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
