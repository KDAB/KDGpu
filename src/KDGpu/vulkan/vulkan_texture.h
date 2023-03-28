#pragma once

#include <KDGpu/api/api_texture.h>

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

struct VulkanTexture : public ApiTexture {
    explicit VulkanTexture(VkImage _image,
                           VmaAllocation _allocation,
                           Format _format,
                           Extent3D _extent,
                           uint32_t _mipLevels,
                           uint32_t _arrayLayers,
                           TextureUsageFlags _usage,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle);

    explicit VulkanTexture(VkImage _image,
                           VmaAllocation _allocation,
                           Format _format,
                           Extent3D _extent,
                           uint32_t _mipLevels,
                           uint32_t _arrayLayers,
                           TextureUsageFlags _usage,
                           bool _ownedBySwapchain,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle);

    VkImage image{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    Format format;
    Extent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    TextureUsageFlags usage;
    bool ownedBySwapchain{ false };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
