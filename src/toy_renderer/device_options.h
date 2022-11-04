#pragma once

#include <toy_renderer/adapter_features.h>

#include <stdint.h>
#include <string>
#include <vector>

namespace ToyRenderer {

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

} // namespace ToyRenderer
