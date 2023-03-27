#pragma once

#include <kdgpu/gpu_core.h>
#include <vector>

namespace KDGpu {

struct PresentOptions;
struct SubmitOptions;

struct ApiQueue {
    virtual void waitUntilIdle() = 0;
    // TODO: Return type and arguments?
    virtual void submit(const SubmitOptions &options) = 0;
    virtual std::vector<PresentResult> present(const PresentOptions &options) = 0;
};

} // namespace KDGpu
