#pragma once

#include <toy_renderer/handle.h>

#include <toy_renderer/toy_renderer_export.h>

namespace ToyRenderer {

class GraphicsApi;

struct Device_t;
struct GraphicsPipeline_t;

class TOY_RENDERER_EXPORT GraphicsPipeline
{
public:
    ~GraphicsPipeline();

    const Handle<GraphicsPipeline_t> &handle() const noexcept { return m_graphicsPipeline; }
    bool isValid() const noexcept { return m_graphicsPipeline.isValid(); }

private:
    explicit GraphicsPipeline(GraphicsApi *api, const Handle<Device_t> &device, const Handle<GraphicsPipeline_t> &graphicsPipeline);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<GraphicsPipeline_t> m_graphicsPipeline;

    friend class Device;
};

} // namespace ToyRenderer
