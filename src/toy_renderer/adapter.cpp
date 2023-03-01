#include "adapter.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>

#include <toy_renderer/api/api_adapter.h>

namespace ToyRenderer {

Adapter::Adapter()
{
}

Adapter::Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter)
    : m_api(api)
    , m_adapter(adapter)
{
}

Adapter::~Adapter()
{
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

AdapterSwapchainProperties Adapter::swapchainProperties(const Surface &surface) const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->querySwapchainProperties(surface.handle());
}

bool Adapter::supportsPresentation(const Surface &surface, uint32_t queueTypeIndex) const noexcept
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->supportsPresentation(surface.handle(), queueTypeIndex);
}

Device Adapter::createDevice(const DeviceOptions &options)
{
    return Device(this, m_api, options);
}

} // namespace ToyRenderer
