#include "adapter.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>

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
        m_properties = m_api->queryAdapterProperties(m_adapter);
        m_propertiesQueried = true;
    }

    return m_properties;
}

const AdapterFeatures &Adapter::features() const noexcept
{
    if (!m_featuresQueried) {
        m_features = m_api->queryAdapterFeatures(m_adapter);
        m_featuresQueried = true;
    }

    return m_features;
}

Device Adapter::createDevice(const DeviceOptions &options)
{
    return Device(m_api, options);
}

} // namespace ToyRenderer
