/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

#include <vector>
#include <spdlog/spdlog.h>

namespace KDGpu {

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
        fmt::format("- currentTransform: {:b}", capabilities.currentTransform),
        fmt::format("- supportedCompositeAlpha: {:b}", capabilities.supportedCompositeAlpha.toInt()),
        fmt::format("- supportedUsageFlags: {:b}", capabilities.supportedUsageFlags.toInt()),
    };
    return fmt::format("SurfaceCapabilities:\n{}", fmt::join(surfaceCapabilitiesString, "\n"));
}

struct SurfaceFormat {
    Format format;
    ColorSpace colorSpace;
};

struct AdapterSwapchainProperties {
    SurfaceCapabilities capabilities;
    std::vector<SurfaceFormat> formats;
    std::vector<PresentMode> presentModes;
};

} // namespace KDGpu
