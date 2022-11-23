#include "instance.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_instance.h>

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

Surface Instance::createSurface(const SurfaceOptions &options)
{
    auto apiInstance = m_api->resourceManager()->getInstance(m_instance);
    return Surface(apiInstance->createSurface(options));
}

} // namespace ToyRenderer
