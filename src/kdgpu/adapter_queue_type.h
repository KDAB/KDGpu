#pragma once

#include <kdgpu/gpu_core.h>

namespace KDGpu {

struct AdapterQueueType {
    bool supportsFeature(QueueFlags featureFlags) const noexcept { return (flags & featureFlags) == featureFlags; }

    QueueFlags flags;
    uint32_t availableQueues;
    uint32_t timestampValidBits;
    Extent3D minImageTransferGranularity;
};

} // namespace KDGpu
