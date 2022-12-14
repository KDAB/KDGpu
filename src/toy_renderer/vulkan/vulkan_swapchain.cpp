#include "vulkan_swapchain.h"

#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanSwapchain::VulkanSwapchain(VkSwapchainKHR _swapchain,
                                 Format _format,
                                 TextureUsageFlags _imageUsageFlags,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle)
    : ApiSwapchain()
    , swapchain(_swapchain)
    , format(_format)
    , imageUsageFlags(_imageUsageFlags)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

std::vector<Handle<Texture_t>> VulkanSwapchain::getTextures()
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (!vulkanDevice) {
        // TODO: Log could not find device
        return {};
    }
    VkDevice device = vulkanDevice->device;

    std::vector<VkImage> vkImages;
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    vkImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, vkImages.data());

    std::vector<Handle<Texture_t>> textureHandles;
    textureHandles.reserve(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i) {
        textureHandles.emplace_back(
                vulkanResourceManager->insertTexture(VulkanTexture(
                        vkImages[i],
                        VK_NULL_HANDLE, // No allocation for swapchain images
                        format,
                        imageUsageFlags,
                        vulkanResourceManager,
                        deviceHandle)));
    }
    return textureHandles;
}

} // namespace ToyRenderer
