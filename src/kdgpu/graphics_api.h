#pragma once

#include <kdgpu/instance.h>
#include <kdgpu/resource_manager.h>
#include <kdgpu/kdgpu_export.h>

namespace KDGpu {

class KDGPU_EXPORT GraphicsApi
{
public:
    virtual ~GraphicsApi();

    Instance createInstance(const InstanceOptions &options = InstanceOptions());

    ResourceManager *resourceManager() noexcept { return m_resourceManager; }
    const ResourceManager *resourceManager() const noexcept { return m_resourceManager; }

protected:
    GraphicsApi();

    ResourceManager *m_resourceManager{ nullptr };
};

} // namespace KDGpu
