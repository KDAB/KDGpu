/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/texture.h>

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
struct KDGPU_EXPORT VulkanTexture {
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
                           const MemoryHandle &_externalMemoryHandle,
                           uint64_t _drmFormatModifier,
                           bool _ownedBySwapchain = false);

    void *map();
    void unmap();

    void hostLayoutTransition(const HostLayoutTransition &transition);
    void copyHostMemoryToTexture(const HostMemoryToTextureCopy &copy);
    void copyTextureToHostMemory(const TextureToHostMemoryCopy &copy);
    void copyTextureToTextureHost(const TextureToTextureCopyHost &copy);

    SubresourceLayout getSubresourceLayout(const TextureSubresource &subresource) const;
    MemoryHandle externalMemoryHandle() const;
    uint64_t drmFormatModifier() const;

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
    MemoryHandle m_externalMemoryHandle{};
    uint64_t m_drmFormatModifier{};
};

} // namespace KDGpu
