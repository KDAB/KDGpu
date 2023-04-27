/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_buffer.h"

#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanBuffer::VulkanBuffer(VkBuffer _buffer,
                           VmaAllocation _allocation,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle)
    : buffer(_buffer)
    , allocation(_allocation)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void *VulkanBuffer::map()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaMapMemory(vulkanDevice->allocator, allocation, &mapped);
    return mapped;
}

void VulkanBuffer::unmap()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaUnmapMemory(vulkanDevice->allocator, allocation);
    mapped = nullptr;
}

} // namespace KDGpu
