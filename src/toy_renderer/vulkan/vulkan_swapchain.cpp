#include "vulkan_swapchain.h"

namespace ToyRenderer {

VulkanSwapchain::VulkanSwapchain(VkSwapchainKHR _swapchain)
    : ApiSwapchain()
    , swapchain(_swapchain)
{
}

} // namespace ToyRenderer
