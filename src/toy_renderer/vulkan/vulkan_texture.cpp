#include "vulkan_texture.h"

namespace ToyRenderer {

VulkanTexture::VulkanTexture(VkImage _image,
                             VmaAllocation _allocation,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle)
    : ApiTexture()
    , image(_image)
    , allocation(allocation)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
