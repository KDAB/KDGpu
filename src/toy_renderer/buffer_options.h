#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct BufferOptions {
    DeviceSize size;
    BufferUsageFlags usage;
    MemoryUsage memoryUsage;
};

} // namespace ToyRenderer
