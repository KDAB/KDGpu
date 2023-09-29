/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_texture.h>

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanTexture
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanTexture : public ApiTexture {
    explicit VulkanTexture(VkImage _image,
                           VmaAllocation _allocation,
                           VmaAllocator _allocator,
                           Format _format,
                           Extent3D _extent,
                           uint32_t _mipLevels,
                           uint32_t _arrayLayers,
                           TextureUsageFlags _usage,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle,
                           const HandleOrFD &_externalMemoryHandle,
                           bool _ownedBySwapchain = false);

    void *map() final;
    void unmap() final;
    SubresourceLayout getSubresourceLayout(const TextureSubresource &subresource) const final;
    HandleOrFD externalMemoryHandle() const final;

    VkImage image{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VmaAllocator allocator{ VK_NULL_HANDLE };
    void *mapped{ nullptr };
    Format format;
    Extent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    TextureUsageFlags usage;
    bool ownedBySwapchain{ false };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    HandleOrFD m_externalMemoryHandle{};
};

} // namespace KDGpu
