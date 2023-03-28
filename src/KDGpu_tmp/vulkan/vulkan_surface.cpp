#include "vulkan_surface.h"

namespace KDGpu {

VulkanSurface::VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance)
    : ApiSurface()
    , surface(_surface)
    , instance(_instance)
{
}

} // namespace KDGpu
