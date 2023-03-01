#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct MemoryBarrier {
    AccessFlags srcMask;
    AccessFlags dstMask;
};

} // namespace ToyRenderer
