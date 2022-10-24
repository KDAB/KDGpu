#include "instance.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>

namespace ToyRenderer {

Instance::Instance(GraphicsApi *api, const InstanceOptions &options)
{
    // Create an instance using the underlying API
    m_api = api;
    m_instance = m_api->resourceManager()->createInstance(options);
}

Instance::~Instance()
{
    // TODO: Destroy the instance using the underlying API
}

std::span<Adapter> Instance::adapters()
{
    if (m_adapters.empty()) {
        auto adapterHandles = m_api->queryAdapters(m_instance);
        const auto adapterCount = static_cast<uint32_t>(adapterHandles.size());
        m_adapters.reserve(adapterCount);
        for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
            m_adapters.emplace_back(Adapter{ m_api, adapterHandles[adapterIndex] });
    }
    return std::span{ m_adapters };
}

} // namespace ToyRenderer
