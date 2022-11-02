#include "vulkan_surface.h"

namespace ToyRenderer {

VulkanSurface::VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance)
    : ApiSurface()
    , surface(_surface)
    , instance(_instance)
{
}

} // namespace ToyRenderer
