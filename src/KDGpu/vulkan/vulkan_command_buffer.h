/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_command_buffer.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct Device_t;
class VulkanResourceManager;

struct KDGPU_EXPORT VulkanCommandBuffer : public ApiCommandBuffer {
    explicit VulkanCommandBuffer(VkCommandBuffer _commandBuffer,
                                 VkCommandPool _commandPool,
                                 VkCommandBufferLevel _commandLevel,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle);

    void begin() final;
    void finish() final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBufferLevel commandLevel{ VK_COMMAND_BUFFER_LEVEL_PRIMARY };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
