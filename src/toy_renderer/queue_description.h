#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>

namespace ToyRenderer {

struct Queue_t;

struct QueueDescription {
    const Handle<Queue_t> queue;
    QueueFlags flags;
    uint32_t timestampValidBits;
    Extent3D minImageTransferGranularity;
    uint32_t queueTypeIndex;
};

} // namespace ToyRenderer
