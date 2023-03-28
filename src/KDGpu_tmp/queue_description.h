#pragma once

#include <kdgpu/gpu_core.h>
#include <kdgpu/handle.h>

namespace KDGpu {

struct Queue_t;

struct QueueDescription {
    const Handle<Queue_t> queue;
    QueueFlags flags;
    uint32_t timestampValidBits;
    Extent3D minImageTransferGranularity;
    uint32_t queueTypeIndex;
};

} // namespace KDGpu
