#pragma once

#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct BufferOptions {
    DeviceSize size;
    BufferUsageFlags usage;
    MemoryUsage memoryUsage;
    SharingMode sharingMode{ SharingMode::Exclusive };
    std::vector<uint32_t> queueTypeIndices{};
};

} // namespace KDGpu
