#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct BindGroupLayout_t;
struct Device_t;
struct BindGroupLayoutOptions;
class GraphicsApi;

class TOY_RENDERER_EXPORT BindGroupLayout
{
public:
    BindGroupLayout();
    ~BindGroupLayout();

    BindGroupLayout(BindGroupLayout &&);
    BindGroupLayout &operator=(BindGroupLayout &&);

    BindGroupLayout(const BindGroupLayout &) = delete;
    BindGroupLayout &operator=(const BindGroupLayout &) = delete;

    const Handle<BindGroupLayout_t> &handle() const noexcept { return m_bindGroupLayout; }
    bool isValid() const noexcept { return m_bindGroupLayout.isValid(); }

    operator Handle<BindGroupLayout_t>() const noexcept { return m_bindGroupLayout; }

private:
    explicit BindGroupLayout(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupLayoutOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroupLayout_t> m_bindGroupLayout;

    friend class Device;
    friend TOY_RENDERER_EXPORT bool operator==(const BindGroupLayout &, const BindGroupLayout &);
};

TOY_RENDERER_EXPORT bool operator==(const BindGroupLayout &a, const BindGroupLayout &b);
TOY_RENDERER_EXPORT bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b);

} // namespace ToyRenderer
