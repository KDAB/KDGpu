#include "bind_group_layout.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_bind_group_layout.h>

namespace ToyRenderer {

BindGroupLayout::BindGroupLayout() = default;
BindGroupLayout::~BindGroupLayout()
{
    if (isValid())
        m_api->resourceManager()->deleteBindGroupLayout(handle());
};

BindGroupLayout::BindGroupLayout(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const Handle<BindGroupLayout_t> &bindGroupLayout)
    : m_api(api)
    , m_device(device)
    , m_bindGroupLayout(bindGroupLayout)
{
}

BindGroupLayout::BindGroupLayout(BindGroupLayout &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_bindGroupLayout = other.m_bindGroupLayout;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_bindGroupLayout = {};
}

BindGroupLayout &BindGroupLayout::operator=(BindGroupLayout &&other)
{
    if (this != &other) {
        m_api = other.m_api;
        m_device = other.m_device;
        m_bindGroupLayout = other.m_bindGroupLayout;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_bindGroupLayout = {};
    }
    return *this;
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
