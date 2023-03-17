#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct ApiFence {
    virtual void wait() = 0;
    virtual void reset() = 0;
    virtual FenceStatus status() = 0;
};

} // namespace ToyRenderer
