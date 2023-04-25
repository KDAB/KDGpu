#include "vulkan_texture.h"

#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanTexture::VulkanTexture(VkImage _image,
                             VmaAllocation _allocation,
                             Format _format,
                             Extent3D _extent,
                             uint32_t _mipLevels,
                             uint32_t _arrayLayers,
                             TextureUsageFlags _usage,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle)
    : ApiTexture()
    , image(_image)
    , allocation(_allocation)
    , format(_format)
    , extent(_extent)
    , mipLevels(_mipLevels)
    , arrayLayers(_arrayLayers)
    , usage(_usage)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

VulkanTexture::VulkanTexture(VkImage _image,
                             VmaAllocation _allocation,
                             Format _format,
                             Extent3D _extent,
                             uint32_t _mipLevels,
                             uint32_t _arrayLayers,
                             TextureUsageFlags _usage,
                             bool _ownedBySwapchain,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle)
    : ApiTexture()
    , image(_image)
    , allocation(_allocation)
    , format(_format)
    , extent(_extent)
    , mipLevels(_mipLevels)
    , arrayLayers(_arrayLayers)
    , usage(_usage)
    , ownedBySwapchain(_ownedBySwapchain)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
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

} // namespace KDGpu
