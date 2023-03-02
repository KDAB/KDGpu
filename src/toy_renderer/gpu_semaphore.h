#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

struct Device_t;
struct GpuSemaphore_t;

class GraphicsApi;

// Reserved for later use
struct GpuSemaphoreOptions {
};

class TOY_RENDERER_EXPORT GpuSemaphore
{
public:
    GpuSemaphore();
    ~GpuSemaphore();

    GpuSemaphore(GpuSemaphore &&);
    GpuSemaphore &operator=(GpuSemaphore &&);

    GpuSemaphore(const GpuSemaphore &) = delete;
    GpuSemaphore &operator=(const GpuSemaphore &) = delete;

    const Handle<GpuSemaphore_t> &handle() const noexcept { return m_gpuSemaphore; }
    bool isValid() const noexcept { return m_gpuSemaphore.isValid(); }

    operator Handle<GpuSemaphore_t>() const noexcept { return m_gpuSemaphore; }

private:
    explicit GpuSemaphore(GraphicsApi *api, const Handle<Device_t> &device, const Handle<GpuSemaphore_t> &gpuSemaphore);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<GpuSemaphore_t> m_gpuSemaphore;

    friend class Device;
};

} // namespace ToyRenderer
