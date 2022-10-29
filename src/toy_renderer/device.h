#pragma once

#include <toy_renderer/device_options.h>
#include <toy_renderer/handle.h>
#include <toy_renderer/queue.h>

#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <string>
#include <vector>

namespace ToyRenderer {

class Adapter;
class GraphicsApi;
struct Adapter_t;
struct Device_t;

class TOY_RENDERER_EXPORT Device
{
public:
    ~Device();

    std::span<Queue> queues() { return m_queues; }

private:
    Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    std::vector<Queue> m_queues;

    friend class Adapter;
};

} // namespace ToyRenderer
