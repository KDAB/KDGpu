#pragma once

#include <toy_renderer/adapter_features.h>
#include <toy_renderer/adapter_properties.h>
#include <toy_renderer/adapter_queue_type.h>
#include <toy_renderer/adapter_swapchain_properties.h>
#include <toy_renderer/handle.h>

#include <vector>

namespace ToyRenderer {

struct Surface_t;

struct ApiAdapter {
    virtual AdapterProperties queryAdapterProperties() = 0;
    virtual AdapterFeatures queryAdapterFeatures() = 0;
    virtual AdapterSwapchainProperties querySwapchainProperties(const Handle<Surface_t> &surfaceHandle) = 0;
    virtual std::vector<AdapterQueueType> queryQueueTypes() = 0;
};

} // namespace ToyRenderer
