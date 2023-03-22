#pragma once

#include <toy_renderer/gpu_core.h>

namespace ToyRenderer {

struct AdapterQueueType {
    bool supportsFeature(QueueFlags featureFlags) const noexcept { return (flags & featureFlags) == featureFlags; }

    QueueFlags flags;
    uint32_t availableQueues;
    uint32_t timestampValidBits;
    Extent3D minImageTransferGranularity;
};

} // namespace ToyRenderer
