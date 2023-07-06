/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/utils/logging.h>

#include <vector>

namespace KDGpu {

/*! \addtogroup public
 *  @{
 */

/**
    @headerfile adapter_swapchain_properties.h <KDGpu/adapter_swapchain_properties.h>
 */
struct SurfaceCapabilities {
    uint32_t minImageCount;
    uint32_t maxImageCount;
    Extent2D currentExtent;
    Extent2D minImageExtent;
    Extent2D maxImageExtent;
    uint32_t maxImageArrayLayers;
    SurfaceTransformFlags supportedTransforms;
    SurfaceTransformFlagBits currentTransform;
    CompositeAlphaFlags supportedCompositeAlpha;
    TextureUsageFlags supportedUsageFlags;
};

/**
    @brief Returns a suitable image count for a given surface, which can be used for minImageCount in SwapchainOptions.
 */
inline uint32_t getSuitableImageCount(const SurfaceCapabilities &capabilities)
{
    if (capabilities.maxImageCount != 0) {
        return std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
    } else {
        return capabilities.minImageCount + 1;
    }
}

/**
    @headerfile adapter_swapchain_properties.h <KDGpu/adapter_swapchain_properties.h>
 */
inline std::string surfaceCapabilitiesToString(const SurfaceCapabilities &capabilities)
{
    const std::vector<std::string> surfaceCapabilitiesString = {
        fmt::format("- minImageCount: {}", capabilities.minImageCount),
        fmt::format("- maxImageCount: {}", capabilities.maxImageCount),
        fmt::format("- currentExtent: {} x {}", capabilities.currentExtent.width, capabilities.currentExtent.height),
        fmt::format("- minImageExtent: {} x {}", capabilities.minImageExtent.width, capabilities.minImageExtent.height),
        fmt::format("- maxImageExtent: {} x {}", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height),
        fmt::format("- maxImageArrayLayers: {}", capabilities.maxImageArrayLayers),
        fmt::format("- supportedTransforms: {:b}", capabilities.supportedTransforms.toInt()),
        fmt::format("- currentTransform: {}", KDGpu::surfaceTransformFlagBitsToString(capabilities.currentTransform)),
        fmt::format("- supportedCompositeAlpha: {:b}", capabilities.supportedCompositeAlpha.toInt()),
        fmt::format("- supportedUsageFlags: {:b}", capabilities.supportedUsageFlags.toInt()),
    };
    return fmt::format("SurfaceCapabilities:\n{}", fmt::join(surfaceCapabilitiesString, "\n"));
}

/**
    @headerfile adapter_swapchain_properties.h <KDGpu/adapter_swapchain_properties.h>
 */
struct SurfaceFormat {
    Format format;
    ColorSpace colorSpace;
};

/**
    @headerfile adapter_swapchain_properties.h <KDGpu/adapter_swapchain_properties.h>
 */
struct AdapterSwapchainProperties {
    SurfaceCapabilities capabilities;
    std::vector<SurfaceFormat> formats;
    std::vector<PresentMode> presentModes;
};

/*! @} */

} // namespace KDGpu
