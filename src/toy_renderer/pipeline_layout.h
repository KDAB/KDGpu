#pragma once

#include <toy_renderer/toy_renderer_export.h>

#include <toy_renderer/handle.h>

namespace ToyRenderer {

class GraphicsApi;

struct Device_t;
struct PipelineLayout_t;

class TOY_RENDERER_EXPORT PipelineLayout
{
public:
    ~PipelineLayout();

    const Handle<PipelineLayout_t> &handle() const noexcept { return m_pipelineLayout; }
    bool isValid() const noexcept { return m_pipelineLayout.isValid(); }

private:
    explicit PipelineLayout(GraphicsApi *api,
                            const Handle<Device_t> &device,
                            const Handle<PipelineLayout_t> &pipelineLayout);

    GraphicsApi *m_api{ nullptr };
    Handle<PipelineLayout_t> m_pipelineLayout;
    Handle<Device_t> m_device;

    friend class Device;
};

} // namespace ToyRenderer
