/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_swapchain.h"

#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <limits>

namespace KDGpu {

VulkanSwapchain::VulkanSwapchain(VkSwapchainKHR _swapchain,
                                 Format _format,
                                 Extent3D _extent,
                                 uint32_t _arrayLayers,
                                 TextureUsageFlags _imageUsageFlags,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle)
    : swapchain(_swapchain)
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
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid device");
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
                        VK_NULL_HANDLE, // No allocator for swapchain images
                        format,
                        extent,
                        1,
                        arrayLayers,
                        imageUsageFlags,
                        vulkanResourceManager,
                        deviceHandle,
                        MemoryHandle{},
                        true // owned by swapchain
                        )));
    }
    return textureHandles;
}

AcquireImageResult VulkanSwapchain::getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore)
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (!vulkanDevice) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid device");
        return AcquireImageResult::DeviceLost;
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

    auto mapVkResultToAcquireImageResult = [](const VkResult r) {
        switch (r) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return AcquireImageResult::OutOfMemory;
        case VK_ERROR_DEVICE_LOST:
            return AcquireImageResult::DeviceLost;
        case VK_ERROR_OUT_OF_DATE_KHR:
            return AcquireImageResult::OutOfDate;
        case VK_ERROR_SURFACE_LOST_KHR:
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return AcquireImageResult::SurfaceLost;
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return AcquireImageResult::ValidationFailed;
        case VK_NOT_READY:
            return AcquireImageResult::NotReady;
        case VK_SUBOPTIMAL_KHR:
            return AcquireImageResult::SubOptimal;
        case VK_SUCCESS:
            return AcquireImageResult::Success;
        case VK_ERROR_UNKNOWN:
        default:
            return AcquireImageResult::Unknown;
        }
    };

    return mapVkResultToAcquireImageResult(result);
}

} // namespace KDGpu
