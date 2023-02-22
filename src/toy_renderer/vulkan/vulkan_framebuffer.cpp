#include "vulkan_framebuffer.h"

namespace ToyRenderer {

ToyRenderer::VulkanFramebuffer::VulkanFramebuffer(VkFramebuffer _framebuffer)
    : ApiFramebuffer()
    , framebuffer(_framebuffer)
{
}

} // namespace ToyRenderer
