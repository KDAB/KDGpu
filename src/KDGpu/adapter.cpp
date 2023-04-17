#include "adapter.h"

#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>

#include <KDGpu/api/api_adapter.h>

namespace KDGpu {

Adapter::Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter)
    : m_api(api)
    , m_adapter(adapter)
{
}

Adapter::~Adapter()
{
    if (isValid())
        m_api->resourceManager()->removeAdapter(m_adapter);
}

Adapter::Adapter(Adapter &&other)
{
    m_api = other.m_api;
    m_adapter = other.m_adapter;

    other.m_api = nullptr;
    other.m_adapter = {};
}

Adapter &Adapter::operator=(Adapter &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->removeAdapter(m_adapter);

        m_api = other.m_api;
        m_adapter = other.m_adapter;

        other.m_api = nullptr;
        other.m_adapter = {};
    }
    return *this;
}

std::vector<Extension> Adapter::extensions() const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->extensions();
}

const AdapterProperties &Adapter::properties() const noexcept
{
    if (!m_propertiesQueried) {
        auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
        m_properties = apiAdapter->queryAdapterProperties();
        m_propertiesQueried = true;
    }

    return m_properties;
}

const AdapterFeatures &Adapter::features() const noexcept
{
    if (!m_featuresQueried) {
        auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
        m_features = apiAdapter->queryAdapterFeatures();
        m_featuresQueried = true;
    }

    return m_features;
}

std::span<AdapterQueueType> Adapter::queueTypes() const
{
    if (m_queueTypes.empty()) {
        // TODO: query queue type information
        auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
        m_queueTypes = apiAdapter->queryQueueTypes();
    }

    return m_queueTypes;
}

AdapterSwapchainProperties Adapter::swapchainProperties(const Handle<Surface_t> &surface) const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->querySwapchainProperties(surface);
}

bool Adapter::supportsPresentation(const Handle<Surface_t> &surface, uint32_t queueTypeIndex) const noexcept
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->supportsPresentation(surface, queueTypeIndex);
}

FormatProperties Adapter::formatProperties(Format format) const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->formatProperties(format);
}

Device Adapter::createDevice(const DeviceOptions &options)
{
    return Device(this, m_api, options);
}

} // namespace KDGpu
