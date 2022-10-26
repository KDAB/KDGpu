#pragma once

#include <toy_renderer/adapter_features.h>
#include <toy_renderer/adapter_properties.h>

namespace ToyRenderer {

struct DeviceOptions;

struct ApiAdapter {
    virtual AdapterProperties queryAdapterProperties() = 0;
    virtual AdapterFeatures queryAdapterFeatures() = 0;

    // TODO: Handle<Device_t>
    virtual void createDevice(const DeviceOptions &options) = 0;
};

} // namespace ToyRenderer
