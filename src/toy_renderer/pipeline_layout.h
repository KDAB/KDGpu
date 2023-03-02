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
    PipelineLayout();
    ~PipelineLayout();

    PipelineLayout(PipelineLayout &&);
    PipelineLayout &operator=(PipelineLayout &&);

    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout &operator=(const PipelineLayout &) = delete;

    const Handle<PipelineLayout_t> &handle() const noexcept { return m_pipelineLayout; }
    bool isValid() const noexcept { return m_pipelineLayout.isValid(); }

    operator Handle<PipelineLayout_t>() const noexcept { return m_pipelineLayout; }

private:
    explicit PipelineLayout(GraphicsApi *api,
                            const Handle<Device_t> &device,
                            const Handle<PipelineLayout_t> &pipelineLayout);

    GraphicsApi *m_api{ nullptr };
    Handle<PipelineLayout_t> m_pipelineLayout;
    Handle<Device_t> m_device;

    friend class Device;
    friend TOY_RENDERER_EXPORT bool operator==(const PipelineLayout &, const PipelineLayout &);
};

TOY_RENDERER_EXPORT bool operator==(const PipelineLayout &a, const PipelineLayout &b);
TOY_RENDERER_EXPORT bool operator!=(const PipelineLayout &a, const PipelineLayout &b);

} // namespace ToyRenderer
