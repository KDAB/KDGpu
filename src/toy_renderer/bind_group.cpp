#include "bind_group.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_bind_group.h>

namespace ToyRenderer {

BindGroup::BindGroup()
{
}

BindGroup::~BindGroup()
{
    if (isValid())
        m_api->resourceManager()->deleteBindGroup(handle());
}

BindGroup::BindGroup(BindGroup &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_bindGroup = other.m_bindGroup;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_bindGroup = {};
}

BindGroup &BindGroup::operator=(BindGroup &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteBindGroup(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_bindGroup = other.m_bindGroup;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_bindGroup = {};
    }
    return *this;
}

BindGroup::BindGroup(GraphicsApi *api, const Handle<Device_t> &device, const Handle<BindGroup_t> &bindGroup)
    : m_api(api)
    , m_device(device)
    , m_bindGroup(bindGroup)
{
}

void BindGroup::update(const BindGroupEntry &entry)
{
    auto apiBindGroup = m_api->resourceManager()->getBindGroup(m_bindGroup);
    apiBindGroup->update(entry);
}

bool operator==(const BindGroup &a, const BindGroup &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_bindGroup == b.m_bindGroup;
}

bool operator!=(const BindGroup &a, const BindGroup &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
