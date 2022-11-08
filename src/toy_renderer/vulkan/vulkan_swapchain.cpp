#include "vulkan_swapchain.h"

#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanSwapchain::VulkanSwapchain(VkSwapchainKHR _swapchain,
                                 VkDevice _device,
                                 VulkanResourceManager *_vulkanResourceManager)
    : ApiSwapchain()
    , vulkanResourceManager(_vulkanResourceManager)
    , swapchain(_swapchain)
    , device(_device)
{
}

std::vector<Handle<Texture_t>> VulkanSwapchain::getTextures()
{
    std::vector<VkImage> vkImages;
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    vkImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, vkImages.data());

    std::vector<Handle<Texture_t>> textureHandles;
    textureHandles.reserve(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i) {
        textureHandles.emplace_back(
                vulkanResourceManager->insertTexture({ vkImages[i],
                                                       device,
                                                       vulkanResourceManager }));
    }
    return textureHandles;
}

} // namespace ToyRenderer
