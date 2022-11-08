#pragma once

#include <toy_renderer/api/api_swapchain.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct VulkanSwapchain : public ApiSwapchain {
    VulkanSwapchain(VkSwapchainKHR _swapchain,
                    VkDevice _device,
                    VulkanResourceManager *_vulkanResourceManager);

    std::vector<Handle<Texture_t>> getTextures() final;

    VulkanResourceManager *vulkanResourceManager{ nullptr };
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
