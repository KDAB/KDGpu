#pragma once

#include <toy_renderer/adapter_features.h>
#include <toy_renderer/adapter_properties.h>
#include <toy_renderer/adapter_queue_type.h>
#include <toy_renderer/adapter_swapchain_properties.h>
#include <toy_renderer/device.h>
#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

#include <span>
#include <string>
#include <vector>

namespace ToyRenderer {

class GraphicsApi;
class Surface;

struct Adapter_t;
struct Instance_t;

struct AdapterOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class TOY_RENDERER_EXPORT Adapter
{
public:
    Adapter();
    ~Adapter();

    Handle<Adapter_t> handle() const noexcept { return m_adapter; }
    bool isValid() const noexcept { return m_adapter.isValid(); }

    const AdapterProperties &properties() const noexcept;
    const AdapterFeatures &features() const noexcept;
    std::span<AdapterQueueType> queueTypes();

    // TODO: Need an api-agnostic way to pass in a surface
    AdapterSwapchainProperties swapchainProperties(const Surface &surface) const;

    Device createDevice(const DeviceOptions &options = DeviceOptions());

private:
    explicit Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter);

    GraphicsApi *m_api{ nullptr };
    Handle<Adapter_t> m_adapter;

    mutable AdapterProperties m_properties;
    mutable bool m_propertiesQueried{ false };

    mutable AdapterFeatures m_features;
    mutable bool m_featuresQueried{ false };

    mutable std::vector<AdapterQueueType> m_queueTypes;

    friend class Instance;
};

} // namespace ToyRenderer
