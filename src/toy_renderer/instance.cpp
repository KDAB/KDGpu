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

std::vector<Extension> Instance::extensions() const
{
    auto apiInstance = m_api->resourceManager()->getInstance(m_instance);
    return apiInstance->extensions();
}

AdapterAndDevice Instance::createDefaultDevice(const Surface &surface,
                                               AdapterDeviceType deviceType) const
{
    // Enumerate the adapters (physical devices) and select one to use. Here we look for
    // a discrete GPU. In a real app, we could fallback to an integrated one.
    Adapter *selectedAdapter = selectAdapter(deviceType);
    if (!selectedAdapter) {
        SPDLOG_CRITICAL("Unable to find a suitable Adapter. Aborting...");
        return {};
    }

    auto queueTypes = selectedAdapter->queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
    SPDLOG_INFO("Queue family 0 graphics and compute support: {}", hasGraphicsAndCompute);

    // We are now able to query the adapter for swapchain properties and presentation support with the window surface
    const auto swapchainProperties = selectedAdapter->swapchainProperties(surface);
    SPDLOG_INFO("Supported swapchain present modes:");
    for (const auto &mode : swapchainProperties.presentModes) {
        SPDLOG_INFO("  - {}", presentModeToString(mode));
    }

    const bool supportsPresentation = selectedAdapter->supportsPresentation(surface, 0); // Query about the 1st queue type
    SPDLOG_INFO("Queue family 0 supports presentation: {}", supportsPresentation);

    const auto adapterExtensions = selectedAdapter->extensions();
    SPDLOG_DEBUG("Supported adapter extensions:");
    for (const auto &extension : adapterExtensions) {
        SPDLOG_DEBUG("  - {} Version {}", extension.name, extension.version);
    }

    if (!supportsPresentation || !hasGraphicsAndCompute) {
        SPDLOG_CRITICAL("Selected adapter queue family 0 does not meet requirements. Aborting.");
        return {};
    }

    // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    auto device = selectedAdapter->createDevice(DeviceOptions{
            .requestedFeatures = selectedAdapter->features() });

    return { selectedAdapter, std::move(device) };
}

std::vector<Adapter *> Instance::adapters() const
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

    std::vector<Adapter *> adapterPtrs;
    adapterPtrs.resize(m_adapters.size());
    for (size_t i = 0, m = m_adapters.size(); i < m; ++i)
        adapterPtrs[i] = m_adapters.data() + i;

    return adapterPtrs;
}

Adapter *Instance::selectAdapter(AdapterDeviceType deviceType) const
{
    std::vector<Adapter *> adaptersList = adapters();
    for (Adapter *adapter : adaptersList) {
        const AdapterProperties &properties = adapter->properties();
        if (properties.deviceType == deviceType)
            return adapter;
    }
    return nullptr;
}

Surface Instance::createSurface(const SurfaceOptions &options)
{
    auto apiInstance = m_api->resourceManager()->getInstance(m_instance);
    return Surface(m_api, apiInstance->createSurface(options));
}

} // namespace ToyRenderer
