#include "vulkan_texture.h"

namespace ToyRenderer {

VulkanTexture::VulkanTexture(VkImage _image,
                             VkDevice _device,
                             VulkanResourceManager *_vulkanResourceManager)
    : ApiTexture()
    , vulkanResourceManager(_vulkanResourceManager)
    , image(_image)
    , device(_device)
{
}

} // namespace ToyRenderer
