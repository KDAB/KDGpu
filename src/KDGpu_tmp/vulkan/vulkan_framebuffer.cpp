#include "vulkan_framebuffer.h"

namespace KDGpu {

KDGpu::VulkanFramebuffer::VulkanFramebuffer(VkFramebuffer _framebuffer)
    : ApiFramebuffer()
    , framebuffer(_framebuffer)
{
}

} // namespace KDGpu
