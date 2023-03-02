#include "instance.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_instance.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace ToyRenderer {

Instance::Instance()
{
}

Instance::Instance(GraphicsApi *api, const InstanceOptions &options)
{
    // Create an instance using the underlying API
    m_api = api;
    m_instance = m_api->resourceManager()->createInstance(options);
}

Instance::~Instance()
{
    if (isValid())
        m_api->resourceManager()->deleteInstance(handle());
}

Instance::Instance(Instance &&other)
{
    m_api = other.m_api;
    m_instance = other.m_instance;
    m_adapters = std::move(other.m_adapters);

    other.m_api = nullptr;
    other.m_instance = {};
    other.m_adapters = {};
}

Instance &Instance::operator=(Instance &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteInstance(handle());

        m_api = other.m_api;
        m_instance = other.m_instance;
        m_adapters = std::move(other.m_adapters);

        other.m_api = nullptr;
        other.m_instance = {};
        other.m_adapters = {};
    }
    return *this;
}

AdapterAndDevice Instance::createDefaultDevice(const Surface &surface,
                                               AdapterDeviceType deviceType) const
{
    // Enumerate the adapters (physical devices) and select one to use. Here we look for
    // a discrete GPU. In a real app, we could fallback to an integrated one.
    Adapter selectedAdapter = selectAdapter(deviceType).value_or(Adapter());
    if (!selectedAdapter.isValid()) {
        spdlog::critical("Unable to find a suitable Adapter. Aborting...");
        return {};
    }

    // We can easily query the adapter for various features, properties and limits.
    spdlog::critical("maxBoundDescriptorSets = {}", selectedAdapter.properties().limits.maxBoundDescriptorSets);
    spdlog::critical("multiDrawIndirect = {}", selectedAdapter.features().multiDrawIndirect);

    auto queueTypes = selectedAdapter.queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
    spdlog::critical("Queue family 0 graphics and compute support: {}", hasGraphicsAndCompute);

    // We are now able to query the adapter for swapchain properties and presentation support with the window surface
    const auto swapchainProperties = selectedAdapter.swapchainProperties(surface);
    const bool supportsPresentation = selectedAdapter.supportsPresentation(surface, 0); // Query about the 1st queue type
    spdlog::critical("Queue family 0 supports presentation: {}", supportsPresentation);

    if (!supportsPresentation || !hasGraphicsAndCompute) {
        spdlog::critical("Selected adapter queue family 0 does not meet requirements. Aborting.");
        return {};
    }

    // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    auto device = selectedAdapter.createDevice();

    return { selectedAdapter, std::move(device) };
}

std::span<Adapter> Instance::adapters() const
{
    if (m_adapters.empty()) {
        auto apiInstance = m_api->resourceManager()->getInstance(m_instance);
        // TODO: If we could look up a handle from a value, we would not need to pass m_instance into
        // queryAdapters(). It is needed so the adapter can store the instance handle for later use
        // when a device needs it to create a VMA allocator.
        auto adapterHandles = apiInstance->queryAdapters(m_instance);
        const auto adapterCount = static_cast<uint32_t>(adapterHandles.size());
        m_adapters.reserve(adapterCount);
        for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
            m_adapters.emplace_back(Adapter{ m_api, adapterHandles[adapterIndex] });
    }
    return std::span{ m_adapters };
}

std::optional<Adapter> Instance::selectAdapter(AdapterDeviceType deviceType) const
{
    const auto &adaptersList = adapters();
    for (const Adapter &adapter : adaptersList) {
        const AdapterProperties &properties = adapter.properties();

        if (properties.deviceType == deviceType)
            return { adapter };
    }
    return {};
}

Surface Instance::createSurface(const SurfaceOptions &options)
{
    auto apiInstance = m_api->resourceManager()->getInstance(m_instance);
    return Surface(m_api, apiInstance->createSurface(options));
}

} // namespace ToyRenderer
