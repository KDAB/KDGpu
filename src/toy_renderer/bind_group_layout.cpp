#include "bind_group_layout.h"

namespace ToyRenderer {

BindGroupLayout::BindGroupLayout() = default;
BindGroupLayout::~BindGroupLayout() = default;

BindGroupLayout::BindGroupLayout(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const Handle<BindGroupLayout_t> &bindGroupLayout)
    : m_api(api)
    , m_device(device)
    , m_bindGroupLayout(bindGroupLayout)
{
}

bool operator==(const BindGroupLayout &a, const BindGroupLayout &b)
{
    return (a.m_api == b.m_api && a.m_device == b.m_device && a.m_bindGroupLayout == b.m_bindGroupLayout);
}

bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b)
{
    return !(a == b);
}

} // namespace ToyRenderer
