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
                           VmaAllocator _allocator,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle,
                           const MemoryHandle &_externalMemoryHandle,
                           const BufferDeviceAddress &_deviceAddress)
    : buffer(_buffer)
    , allocation(_allocation)
    , allocator(_allocator)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , m_externalMemoryHandle(_externalMemoryHandle)
    , m_bufferAddress(_deviceAddress)
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

// Note: invalidating before mapping is only needed on non host coherent memory
// (AMD, Intel, NVIDIA) driver currently provide HOST_COHERENT flag on all memory types that are HOST_VISIBLE
void VulkanBuffer::invalidate()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaInvalidateAllocation(vulkanDevice->allocator, allocation, 0, VK_WHOLE_SIZE);
}

// Note: flushing after mapping is only needed on non host coherent memory
// (AMD, Intel, NVIDIA) driver currently provide HOST_COHERENT flag on all memory types that are HOST_VISIBLE
void VulkanBuffer::flush()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaFlushAllocation(vulkanDevice->allocator, allocation, 0, VK_WHOLE_SIZE);
}

MemoryHandle VulkanBuffer::externalMemoryHandle() const
{
    return m_externalMemoryHandle;
}

BufferDeviceAddress VulkanBuffer::bufferDeviceAddress() const
{
    return m_bufferAddress;
}

} // namespace KDGpu
