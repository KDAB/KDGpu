#include "vulkan_texture.h"

namespace ToyRenderer {

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

} // namespace ToyRenderer
