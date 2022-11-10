#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct Texture_t;

struct TextureViewOptions {
    ViewType viewType{ ViewType::ViewType2D };
    Format format{ Format::UNDEFINED };
    TextureSubresourceRange range{};
};

} // namespace ToyRenderer
