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
                          VulkanResourceManager *_vulkanResourceManager,
                          const Handle<Device_t> &_deviceHandle,
                          const HandleOrFD &_externalMemoryHandle);

    void *map() final;
    void unmap() final;
    void invalidate() final;
    void flush() final;
    HandleOrFD externalMemoryHandle() const final;

    VkBuffer buffer{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    void *mapped{ nullptr };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    HandleOrFD m_externalMemoryHandle{};
};

} // namespace KDGpu
