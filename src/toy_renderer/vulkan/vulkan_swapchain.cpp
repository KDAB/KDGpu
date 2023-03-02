#include "vulkan_swapchain.h"

#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <limits>

namespace ToyRenderer {

VulkanSwapchain::VulkanSwapchain(VkSwapchainKHR _swapchain,
                                 Format _format,
                                 Extent3D _extent,
                                 uint32_t _arrayLayers,
                                 TextureUsageFlags _imageUsageFlags,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle)
    : ApiSwapchain()
    , swapchain(_swapchain)
    , format(_format)
    , extent(_extent)
    , arrayLayers(_arrayLayers)
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
                        extent,
                        1,
                        arrayLayers,
                        imageUsageFlags,
                        true, // owned by swapchain
                        vulkanResourceManager,
                        deviceHandle)));
    }
    return textureHandles;
}

bool VulkanSwapchain::getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore)
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (!vulkanDevice) {
        // TODO: Log could not find device
        return false;
    }
    VkDevice device = vulkanDevice->device;

    VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
    if (semaphore.isValid()) {
        VulkanGpuSemaphore *vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(semaphore);
        if (vulkanSemaphore)
            vkSemaphore = vulkanSemaphore->semaphore;
    }

    const VkResult result = vkAcquireNextImageKHR(
            device, swapchain, std::numeric_limits<uint64_t>::max(),
            vkSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        return true;
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // TODO: Indicate we need to resize
        return false;
    }
    return false;
}

} // namespace ToyRenderer
