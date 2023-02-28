#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct Device_t;
struct ComputePipeline_t;

class TOY_RENDERER_EXPORT ComputePipeline
{
public:
    ComputePipeline();
    ~ComputePipeline();

    const Handle<ComputePipeline_t> &handle() const noexcept { return m_computePipeline; }
    bool isValid() const noexcept { return m_computePipeline.isValid(); }

    operator Handle<ComputePipeline_t>() const noexcept { return m_computePipeline; }

private:
    explicit ComputePipeline(GraphicsApi *api,
                             const Handle<Device_t> &device,
                             const Handle<ComputePipeline_t> &computePipeline);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ComputePipeline_t> m_computePipeline;

    friend class Device;
    friend TOY_RENDERER_EXPORT bool operator==(const ComputePipeline &, const ComputePipeline &);
};

TOY_RENDERER_EXPORT bool operator==(const ComputePipeline &a, const ComputePipeline &b);
TOY_RENDERER_EXPORT bool operator!=(const ComputePipeline &a, const ComputePipeline &b);

} // namespace ToyRenderer
