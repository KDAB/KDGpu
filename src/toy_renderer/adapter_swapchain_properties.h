#pragma once

#include <toy_renderer/gpu_core.h>

#include <vector>

namespace ToyRenderer {

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

struct SurfaceFormat {
    Format format;
    ColorSpace colorSpace;
};

struct AdapterSwapchainProperties {
    SurfaceCapabilities capabilities;
    std::vector<SurfaceFormat> formats;
    std::vector<PresentMode> presentModes;
};

} // namespace ToyRenderer
