#include "vulkan_surface.h"

namespace KDGpu {

VulkanSurface::VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance, bool _isOwned)
    : ApiSurface()
    , surface(_surface)
    , instance(_instance)
    , isOwned(_isOwned)
{
}

} // namespace KDGpu
