#pragma once

#include <toy_renderer/bind_group_description.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct BindGroupEntry;
struct BindGroup_t;
struct Device_t;
struct BindGroupOptions;
class GraphicsApi;

// A BindGroup is what is known as a descriptor set in Vulkan parlance. Other APIs such
// as web-gpu call them bind groups which to me helps with the mental model a little more.
//

class TOY_RENDERER_EXPORT BindGroup
{
public:
    BindGroup();
    ~BindGroup();

    BindGroup(BindGroup &&);
    BindGroup &operator=(BindGroup &&);

    BindGroup(const BindGroup &) = delete;
    BindGroup &operator=(const BindGroup &) = delete;

    const Handle<BindGroup_t> &handle() const noexcept { return m_bindGroup; }
    bool isValid() const noexcept { return m_bindGroup.isValid(); }

    operator Handle<BindGroup_t>() const noexcept { return m_bindGroup; }

    void update(const BindGroupEntry &entry);

private:
    explicit BindGroup(GraphicsApi *api, const Handle<Device_t> &device, const BindGroupOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<BindGroup_t> m_bindGroup;

    friend class Device;
    friend TOY_RENDERER_EXPORT bool operator==(const BindGroup &, const BindGroup &);
};

TOY_RENDERER_EXPORT bool operator==(const BindGroup &a, const BindGroup &b);
TOY_RENDERER_EXPORT bool operator!=(const BindGroup &a, const BindGroup &b);

} // namespace ToyRenderer
