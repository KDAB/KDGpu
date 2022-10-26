#pragma once

#include <toy_renderer/instance.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class TOY_RENDERER_EXPORT GraphicsApi
{
public:
    virtual ~GraphicsApi();

    Instance createInstance(const InstanceOptions &options = InstanceOptions());

    ResourceManager *resourceManager() noexcept { return m_resourceManager; }
    const ResourceManager *resourceManager() const noexcept { return m_resourceManager; }

protected:
    GraphicsApi();

    virtual std::vector<Handle<Adapter_t>> queryAdapters(const Handle<Instance_t> &instanceHandle) = 0;

    ResourceManager *m_resourceManager{ nullptr };

    friend class Instance;
    friend class Adapter;
};

} // namespace ToyRenderer
