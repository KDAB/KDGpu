/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_swapchain.h"

#include <KDXr/openxr/openxr_enums.h>
#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

#include <KDGpu/graphics_api.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

namespace KDXr {

OpenXrSwapchain::OpenXrSwapchain(OpenXrResourceManager *_openxrResourceManager,
                                 XrSwapchain _swapchain,
                                 const Handle<Session_t> &_sessionHandle,
                                 const SwapchainOptions &_options) noexcept
    : ApiSwapchain()
    , openxrResourceManager(_openxrResourceManager)
    , swapchain(_swapchain)
    , sessionHandle(_sessionHandle)
    , options(_options)
{
}

std::vector<KDGpu::Texture> OpenXrSwapchain::getTextures()
{
    OpenXrSession *openXrSession = openxrResourceManager->getSession(sessionHandle);
    assert(openXrSession);
    auto graphicsApi = openXrSession->graphicsApi;
    if (auto vulkanApi = dynamic_cast<KDGpu::VulkanGraphicsApi *>(graphicsApi)) {
        // Query the number of images in the swapchain
        uint32_t imageCount = 0;
        if (xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate swapchain image count.");
            return {};
        }
        SPDLOG_LOGGER_INFO(Logger::logger(), "Color swapchain image count: {}", imageCount);

        std::vector<XrSwapchainImageVulkanKHR> swapchainImages(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
        if (xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount, reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImages.data())) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate swapchain images.");
            return {};
        }

        std::vector<KDGpu::Texture> textures;
        textures.reserve(imageCount);
        for (uint32_t j = 0; j < imageCount; ++j) {
            KDGpu::TextureUsageFlags usageFlags = kdxrSwapchainUsageFlagsToKDGpuTextureUsageFlags(options.usage);
            // clang-format off
            const KDGpu::TextureOptions textureOptions = {
                .type = KDGpu::TextureType::TextureType2D,
                .format = options.format,
                .extent = {
                        .width = options.width,
                        .height = options.height,
                        .depth = 1
                },
                .mipLevels = options.mipLevels,
                .arrayLayers = options.arrayLayers,
                .samples = static_cast<KDGpu::SampleCountFlagBits>(options.sampleCount),
                .usage = usageFlags,
                .memoryUsage = KDGpu::MemoryUsage::GpuOnly
            };
            // clang-format on
            KDGpu::Texture texture = vulkanApi->createTextureFromExistingVkImage(openXrSession->deviceHandle, textureOptions, swapchainImages[j].image);
            textures.emplace_back(std::move(texture));
        }

        return textures;
    }

    SPDLOG_LOGGER_ERROR(Logger::logger(), "Unsupported graphics API");
    return {};
}

} // namespace KDXr
