#include "vulkan_surface.h"

namespace ToyRenderer {

VulkanSurface::VulkanSurface(VkSurfaceKHR _surface)
    : ApiSurface()
    , surface(_surface)
{
}

} // namespace ToyRenderer
