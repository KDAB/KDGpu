#pragma once

#include <toy_renderer/adapter_features.h>
#include <toy_renderer/adapter_properties.h>
#include <toy_renderer/adapter_queue_type.h>

#include <vector>

namespace ToyRenderer {

struct ApiAdapter {
    virtual AdapterProperties queryAdapterProperties() = 0;
    virtual AdapterFeatures queryAdapterFeatures() = 0;
    virtual std::vector<AdapterQueueType> queryQueueTypes() = 0;
};

} // namespace ToyRenderer
