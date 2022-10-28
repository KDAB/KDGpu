#pragma once

#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

#include <string>
#include <vector>

namespace ToyRenderer {

class GraphicsApi;
struct Adapter_t;
struct Device_t;

struct QueueRequest {
    uint32_t familyIndex;
    uint32_t count;
    std::vector<float> priorities;
};

struct DeviceOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
    std::vector<QueueRequest> queues;
};

class TOY_RENDERER_EXPORT Device
{
public:
    ~Device();

private:
    Device(GraphicsApi *api, const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;

    friend class Adapter;
};

} // namespace ToyRenderer
