#pragma once

#include <kdgpu/gpu_core.h>

namespace KDGpu {

struct Texture_t;

struct TextureViewOptions {
    ViewType viewType{ ViewType::ViewType2D };
    Format format{ Format::UNDEFINED };
    TextureSubresourceRange range{};
};

} // namespace KDGpu
