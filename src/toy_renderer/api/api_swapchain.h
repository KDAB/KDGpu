#pragma once

#include <toy_renderer/handle.h>

#include <vector>

namespace ToyRenderer {

struct GpuSemaphore_t;
struct Texture_t;
struct ApiSwapchain {
    virtual std::vector<Handle<Texture_t>> getTextures() = 0;
    virtual bool getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore) = 0;
};

} // namespace ToyRenderer
