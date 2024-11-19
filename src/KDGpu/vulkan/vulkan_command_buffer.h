/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct Buffer_t;
struct Device_t;
class VulkanResourceManager;

/**
 * @brief VulkanCommandBuffer
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanCommandBuffer {
    explicit VulkanCommandBuffer(VkCommandBuffer _commandBuffer,
                                 VkCommandPool _commandPool,
                                 VkCommandBufferLevel _commandLevel,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle);

    void begin();
    void finish();

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBufferLevel commandLevel{ VK_COMMAND_BUFFER_LEVEL_PRIMARY };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    std::vector<Handle<Buffer_t>> temporaryBuffersToRelease;
};

} // namespace KDGpu
