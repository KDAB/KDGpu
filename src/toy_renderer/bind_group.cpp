#include "bind_group.h"

namespace ToyRenderer {

BindGroup::BindGroup()
{
}

BindGroup::~BindGroup()
{
}

BindGroup::BindGroup(GraphicsApi *api, const Handle<Device_t> &device, const Handle<BindGroup_t> &bindGroup)
    : m_api(api)
    , m_device(device)
    , m_bindGroup(bindGroup)
{
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
