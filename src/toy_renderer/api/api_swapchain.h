#pragma once

#include <toy_renderer/handle.h>

#include <vector>

namespace ToyRenderer {

struct Texture_t;
struct ApiSwapchain {
    virtual std::vector<Handle<Texture_t>> getTextures() = 0;
};

} // namespace ToyRenderer
