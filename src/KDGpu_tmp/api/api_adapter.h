#pragma once

#include <kdgpu/adapter_features.h>
#include <kdgpu/adapter_properties.h>
#include <kdgpu/adapter_queue_type.h>
#include <kdgpu/adapter_swapchain_properties.h>
#include <kdgpu/gpu_core.h>
#include <kdgpu/handle.h>

#include <vector>

namespace KDGpu {

struct Surface_t;

struct ApiAdapter {
    virtual std::vector<Extension> extensions() const = 0;
    virtual AdapterProperties queryAdapterProperties() = 0;
    virtual AdapterFeatures queryAdapterFeatures() = 0;
    virtual AdapterSwapchainProperties querySwapchainProperties(const Handle<Surface_t> &surfaceHandle) = 0;
    virtual std::vector<AdapterQueueType> queryQueueTypes() = 0;
    virtual bool supportsPresentation(const Handle<Surface_t> surfaceHandle, uint32_t queueTypeIndex) = 0;
    virtual FormatProperties formatProperties(Format format) const = 0;
};

} // namespace KDGpu
