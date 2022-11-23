#pragma once

#include <toy_renderer/gpu_core.h>

#include <vector>

namespace ToyRenderer {

struct BufferOptions {
    DeviceSize size;
    BufferUsageFlags usage;
    MemoryUsage memoryUsage;
    SharingMode sharingMode{ SharingMode::Exclusive };
    std::vector<uint32_t> queueTypeIndices{};
};

} // namespace ToyRenderer
