#pragma once

#include <kdgpu/handle.h>
#include <kdgpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct GpuSemaphore_t;
struct Texture_t;
struct ApiSwapchain {
    virtual std::vector<Handle<Texture_t>> getTextures() = 0;
    virtual AcquireImageResult getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore) = 0;
};

} // namespace KDGpu
