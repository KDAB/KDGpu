#include "instance.h"
#include "resource_manager.h"

namespace Gpu {

Instance::Instance()
{
    // TODO: Create an instance using the underlying API
}

Instance::Instance(const InstanceOptions &options)
{
    // TODO: Create an instance using the underlying API
    m_handle = ResourceManager::instance()->createInstance(options);
}

Instance::~Instance()
{
    // TODO: Destroy the instance using the underlying API
}

Adapter Instance::requestAdapter(const AdapterSettings &settings)
{
    // TODO: Create an adapter using the underlying API
    return Adapter();
}

} // namespace Gpu
