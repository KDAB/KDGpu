#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct BindGroupLayout_t;
struct Device_t;
class GraphicsApi;

class TOY_RENDERER_EXPORT BindGroupLayout
{
public:
    BindGroupLayout();
    ~BindGroupLayout();

    const Handle<BindGroupLayout_t> &handle() const noexcept { return m_bindGroupLayout; }
    bool isValid() const noexcept { return m_bindGroupLayout.isValid(); }

    operator Handle<BindGroupLayout_t>() const noexcept { return m_bindGroupLayout; }

private:
    explicit BindGroupLayout(GraphicsApi *api, const Handle<Device_t> &device, const Handle<BindGroupLayout_t> &bindGroupLayout);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroupLayout_t> m_bindGroupLayout;

    friend class Device;
    friend TOY_RENDERER_EXPORT bool operator==(const BindGroupLayout &, const BindGroupLayout &);
};

TOY_RENDERER_EXPORT bool operator==(const BindGroupLayout &a, const BindGroupLayout &b);
TOY_RENDERER_EXPORT bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b);

} // namespace ToyRenderer
