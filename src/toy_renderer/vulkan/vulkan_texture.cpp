#include "vulkan_texture.h"

namespace ToyRenderer {

VulkanTexture::VulkanTexture(VkImage _image,
                             VmaAllocation _allocation,
                             Format _format,
                             TextureUsageFlags _usage,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle)
    : ApiTexture()
    , image(_image)
    , allocation(allocation)
    , format(_format)
    , usage(_usage)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
