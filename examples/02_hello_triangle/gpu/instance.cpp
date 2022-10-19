#include "instance.h"

#include "resource_manager.h"

namespace Gpu {

Instance::Instance()
{
    // TODO: Create an instance using the underlying API
}

Instance::Instance(const InstanceOptions &options)
{
    // Create an instance using the underlying API
    m_instance = ResourceManager::instance()->createInstance(options);
}

Instance::~Instance()
{
    // TODO: Destroy the instance using the underlying API
}

std::span<Adapter> Instance::adapters()
{
    if (!m_adapters.empty())
        return std::span{ m_adapters };

    const auto adapterCount = ResourceManager::instance()->adapterCount(m_instance);
    m_adapters.reserve(adapterCount);
    for (auto adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
        m_adapters.emplace_back(Adapter(m_instance, adapterIndex));

    return std::span{ m_adapters };
}

} // namespace Gpu
