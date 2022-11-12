#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct TextureOptions {
    TextureType type;
    Format format;
    Extent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers{ 1 };
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit };
    TextureTiling tiling{ TextureTiling::Optimal };
    TextureUsageFlags usage;
    SharingMode sharingMode{ SharingMode::Exclusive };
    std::vector<uint32_t> queueTypeIndices{};
    TextureLayout initialLayout{ TextureLayout::Undefined };
    // TODO: TextureFlags flags;
};

} // namespace ToyRenderer
