#pragma once

#include <kdgpu/adapter_features.h>

#include <stdint.h>
#include <string>
#include <vector>

namespace KDGpu {

struct QueueRequest {
    uint32_t queueTypeIndex;
    uint32_t count;
    std::vector<float> priorities;
};

struct DeviceOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
    std::vector<QueueRequest> queues;
    AdapterFeatures requestedFeatures;
};

} // namespace KDGpu
