#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Device_t;
struct Fence_t;

class GraphicsApi;

struct FenceOptions {
};

class TOY_RENDERER_EXPORT Fence
{
public:
    Fence();
    ~Fence();

    Fence(Fence &&);
    Fence &operator=(Fence &&);

    Fence(const Fence &) = delete;
    Fence &operator=(const Fence &) = delete;

    const Handle<Fence_t> &handle() const noexcept { return m_fence; }
    bool isValid() const noexcept { return m_fence.isValid(); }

    operator Handle<Fence_t>() const noexcept { return m_fence; }

    void reset();
    void wait();

private:
    explicit Fence(GraphicsApi *api, const Handle<Device_t> &device, const Handle<Fence_t> &fence);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<Fence_t> m_fence;

    friend TOY_RENDERER_EXPORT bool operator==(const Fence &, const Fence &);
    friend class Device;
};

TOY_RENDERER_EXPORT bool operator==(const Fence &a, const Fence &b);
TOY_RENDERER_EXPORT bool operator!=(const Fence &a, const Fence &b);

} // namespace ToyRenderer
