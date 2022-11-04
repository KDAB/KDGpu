#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct SwapchainOptions {
    Surface surface;
    Format format{ Format::B8G8R8A8_UNORM };
    ColorSpace colorSpace{ ColorSpace::SRgbNonlinear };
    uint32_t minImageCount{ 3 };
    Extent2D imageExtent;
    uint32_t imageLayers{ 1 };
    ImageUsageFlags imageUsageFlags{ static_cast<ImageUsageFlags>(ImageUsageFlagBits::ColorAttachmentBit) };
    SurfaceTransformFlags transformFlags{ static_cast<SurfaceTransformFlags>(SurfaceTransformFlagBits::IdentityBit) };
    PresentMode presentMode{ PresentMode::Mailbox };
    bool clipped{ true };
};

} // namespace ToyRenderer
