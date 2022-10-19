#include "adapter.h"

#include "resource_manager.h"

namespace Gpu {

Adapter::Adapter(const Handle<Instance_t> &instance, uint32_t adapterIndex)
{
    m_adapter = ResourceManager::instance()->getAdapter(instance, adapterIndex);
}

Adapter::~Adapter()
{
}

} // namespace Gpu
