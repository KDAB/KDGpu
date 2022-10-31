#pragma once

#include <toy_renderer/api/api_swapchain.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanSwapchain : public ApiSwapchain {
    VulkanSwapchain(VkSwapchainKHR _swapchain);

    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
