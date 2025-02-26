/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_texture.h"

#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_enums.h>
namespace KDGpu {

VulkanTexture::VulkanTexture(VkImage _image,
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
                             bool _ownedBySwapchain)
    : image(_image)
    , allocation(_allocation)
    , allocator(_allocator)
    , format(_format)
    , extent(_extent)
    , mipLevels(_mipLevels)
    , arrayLayers(_arrayLayers)
    , usage(_usage)
    , ownedBySwapchain(_ownedBySwapchain)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , m_externalMemoryHandle(_externalMemoryHandle)
    , m_drmFormatModifier(_drmFormatModifier)
{
}

void VulkanTexture::hostLayoutTransition(const HostLayoutTransition &transition)
{
#if defined(VK_EXT_host_image_copy)
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    assert(vulkanDevice->vkTransitionImageLayout != nullptr);

    VkHostImageLayoutTransitionInfoEXT layoutTransition{
        .sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO_EXT,
        .pNext = nullptr,
        .image = this->image,
        .oldLayout = textureLayoutToVkImageLayout(transition.oldLayout),
        .newLayout = textureLayoutToVkImageLayout(transition.newLayout),
        .subresourceRange = {
                .aspectMask = textureAspectFlagsToVkImageAspectFlags(transition.range.aspectMask),
                .baseMipLevel = transition.range.baseMipLevel,
                .levelCount = transition.range.levelCount,
                .baseArrayLayer = transition.range.baseArrayLayer,
                .layerCount = transition.range.layerCount,
        },
    };

    vulkanDevice->vkTransitionImageLayout(vulkanDevice->device, 1, &layoutTransition);
#else
    assert(false);
#endif
}

void VulkanTexture::copyHostMemoryToTexture(const HostMemoryToTextureCopy &copy)
{
#if defined(VK_EXT_host_image_copy)
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    assert(vulkanDevice->vkCopyMemoryToImage != nullptr);

    std::vector<VkMemoryToImageCopyEXT> regions;
    regions.reserve(copy.regions.size());
    for (const HostMemoryToTextureCopyRegion &r : copy.regions) {
        regions.emplace_back(VkMemoryToImageCopyEXT{
                .sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY_EXT,
                .pNext = nullptr,
                .pHostPointer = r.srcHostMemoryPointer,
                .memoryRowLength = static_cast<uint32_t>(r.srcMemoryRowLength),
                .memoryImageHeight = static_cast<uint32_t>(r.srcMemoryImageHeight),
                .imageSubresource = {
                        .aspectMask = textureAspectFlagsToVkImageAspectFlags(r.dstSubresource.aspectMask),
                        .mipLevel = r.dstSubresource.mipLevel,
                        .baseArrayLayer = r.dstSubresource.baseArrayLayer,
                        .layerCount = r.dstSubresource.layerCount,
                },
                .imageOffset = { .x = r.dstOffset.x, .y = r.dstOffset.y, .z = r.dstOffset.z },
                .imageExtent = { .width = r.dstExtent.width, .height = r.dstExtent.height, .depth = r.dstExtent.depth },
        });
    }

    const VkCopyMemoryToImageInfoEXT copyInfo{
        .sType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO_EXT,
        .pNext = nullptr,
        .flags = hostImageCopyFlagsToVkHostImageCopyFlags(copy.flags),
        .dstImage = this->image,
        .dstImageLayout = textureLayoutToVkImageLayout(copy.dstTextureLayout),
        .regionCount = static_cast<uint32_t>(regions.size()),
        .pRegions = regions.data(),
    };

    vulkanDevice->vkCopyMemoryToImage(vulkanDevice->device, &copyInfo);
#else
    assert(false);
#endif
}

void VulkanTexture::copyTextureToHostMemory(const TextureToHostMemoryCopy &copy)
{
#if defined(VK_EXT_host_image_copy)
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    assert(vulkanDevice->vkCopyImageToMemory != nullptr);

    std::vector<VkImageToMemoryCopyEXT> regions;
    regions.reserve(copy.regions.size());
    for (const TextureToHostMemoryCopyRegion &r : copy.regions) {
        regions.emplace_back(VkImageToMemoryCopyEXT{
                .sType = VK_STRUCTURE_TYPE_IMAGE_TO_MEMORY_COPY_EXT,
                .pNext = nullptr,
                .pHostPointer = r.dstHostMemoryPointer,
                .memoryRowLength = static_cast<uint32_t>(r.dstMemoryRowLength),
                .memoryImageHeight = static_cast<uint32_t>(r.dstMemoryImageHeight),
                .imageSubresource = {
                        .aspectMask = textureAspectFlagsToVkImageAspectFlags(r.srcSubresource.aspectMask),
                        .mipLevel = r.srcSubresource.mipLevel,
                        .baseArrayLayer = r.srcSubresource.baseArrayLayer,
                        .layerCount = r.srcSubresource.layerCount,
                },
                .imageOffset = { .x = r.srcOffset.x, .y = r.srcOffset.y, .z = r.srcOffset.z },
                .imageExtent = { .width = r.srcExtent.width, .height = r.srcExtent.height, .depth = r.srcExtent.depth },
        });
    }

    const VkCopyImageToMemoryInfoEXT copyInfo{
        .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO_EXT,
        .pNext = nullptr,
        .flags = hostImageCopyFlagsToVkHostImageCopyFlags(copy.flags),
        .srcImage = this->image,
        .srcImageLayout = textureLayoutToVkImageLayout(copy.textureLayout),
        .regionCount = static_cast<uint32_t>(regions.size()),
        .pRegions = regions.data(),
    };

    vulkanDevice->vkCopyImageToMemory(vulkanDevice->device, &copyInfo);
#else
    assert(false);
#endif
}

void VulkanTexture::copyTextureToTextureHost(const TextureToTextureCopyHost &copy)
{
#if defined(VK_EXT_host_image_copy)
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    assert(vulkanDevice->vkCopyImageToImage != nullptr);

    VulkanTexture *dstVulkanTexture = vulkanResourceManager->getTexture(copy.dstTexture);
    assert(dstVulkanTexture != nullptr);

    std::vector<VkImageCopy2KHR> regions;
    regions.reserve(copy.regions.size());
    for (const TextureToTextureHostCopyRegion &r : copy.regions) {
        regions.emplace_back(VkImageCopy2KHR{
                .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                .pNext = nullptr,
                .srcSubresource = {
                        .aspectMask = textureAspectFlagsToVkImageAspectFlags(r.srcSubresource.aspectMask),
                        .mipLevel = r.srcSubresource.mipLevel,
                        .baseArrayLayer = r.srcSubresource.baseArrayLayer,
                        .layerCount = r.srcSubresource.layerCount,
                },
                .srcOffset = { .x = r.srcOffset.x, .y = r.srcOffset.y, .z = r.srcOffset.z },
                .dstSubresource = {
                        .aspectMask = textureAspectFlagsToVkImageAspectFlags(r.dstSubresource.aspectMask),
                        .mipLevel = r.dstSubresource.mipLevel,
                        .baseArrayLayer = r.dstSubresource.baseArrayLayer,
                        .layerCount = r.dstSubresource.layerCount,
                },
                .dstOffset = { .x = r.dstOffset.x, .y = r.dstOffset.y, .z = r.dstOffset.z },
                .extent = { .width = r.extent.width, .height = r.extent.height, .depth = r.extent.depth },
        });
    }

    const VkCopyImageToImageInfoEXT copyInfo{
        .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_IMAGE_INFO_EXT,
        .pNext = nullptr,
        .flags = hostImageCopyFlagsToVkHostImageCopyFlags(copy.flags),
        .srcImage = this->image,
        .srcImageLayout = textureLayoutToVkImageLayout(copy.textureLayout),
        .dstImage = dstVulkanTexture->image,
        .dstImageLayout = textureLayoutToVkImageLayout(copy.dstTextureLayout),
        .regionCount = static_cast<uint32_t>(regions.size()),
        .pRegions = regions.data(),
    };

    vulkanDevice->vkCopyImageToImage(vulkanDevice->device, &copyInfo);
#else
    assert(false);
#endif
}

void *VulkanTexture::map()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaMapMemory(vulkanDevice->allocator, allocation, &mapped);
    return mapped;
}

void VulkanTexture::unmap()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaUnmapMemory(vulkanDevice->allocator, allocation);
    mapped = nullptr;
}

SubresourceLayout VulkanTexture::getSubresourceLayout(const TextureSubresource &subresource) const
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    VkImageSubresource vkSubresource = {
        .aspectMask = textureAspectFlagsToVkImageAspectFlags(subresource.aspectMask),
        .mipLevel = subresource.mipLevel,
        .arrayLayer = subresource.arrayLayer
    };

    VkSubresourceLayout vkLayout;
    vkGetImageSubresourceLayout(vulkanDevice->device, image, &vkSubresource, &vkLayout);

    SubresourceLayout layout = {
        .offset = vkLayout.offset,
        .size = vkLayout.size,
        .rowPitch = vkLayout.rowPitch,
        .arrayPitch = vkLayout.arrayPitch,
        .depthPitch = vkLayout.depthPitch
    };
    return layout;
}

MemoryHandle VulkanTexture::externalMemoryHandle() const
{
    return m_externalMemoryHandle;
}

uint64_t VulkanTexture::drmFormatModifier() const
{
    return m_drmFormatModifier;
}

} // namespace KDGpu
