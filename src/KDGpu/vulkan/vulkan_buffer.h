/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_buffer.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanBuffer
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanBuffer : public ApiBuffer {
    explicit VulkanBuffer(VkBuffer _buffer,
                          VmaAllocation _allocation,
                          VmaAllocator _allocator,
                          VulkanResourceManager *_vulkanResourceManager,
                          const Handle<Device_t> &_deviceHandle,
                          const MemoryHandle &_externalMemoryHandle,
                          const BufferDeviceAddress &_deviceAddress);

    void *map() final;
    void unmap() final;
    void invalidate() final;
    void flush() final;
    MemoryHandle externalMemoryHandle() const final;
    BufferDeviceAddress bufferDeviceAddress() const final;

    VkBuffer buffer{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VmaAllocator allocator{ VK_NULL_HANDLE };
    void *mapped{ nullptr };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    MemoryHandle m_externalMemoryHandle{};
    BufferDeviceAddress m_bufferAddress{ 0 };
};

} // namespace KDGpu
