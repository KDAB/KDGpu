#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

#include <vector>

namespace ToyRenderer {

struct Buffer_t;
struct Device_t;
struct GraphicsPipeline_t;
struct RenderPassCommandRecorder_t;

class GraphicsApi;

struct DrawCommand {
    uint32_t vertexCount{ 0 };
    uint32_t instanceCount{ 1 };
    uint32_t firstVertex{ 0 };
    uint32_t firstInstance{ 0 };
};

class TOY_RENDERER_EXPORT RenderPassCommandRecorder
{
public:
    ~RenderPassCommandRecorder();

    void setPipeline(const Handle<GraphicsPipeline_t> &pipeline);

    // TODO: Add overload for setting many vertex buffers at once
    void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer);

    void draw(const DrawCommand &drawCmd);
    void draw(const std::vector<DrawCommand> &drawCmds);

    void end();

private:
    explicit RenderPassCommandRecorder(GraphicsApi *api,
                                       const Handle<Device_t> &device,
                                       const Handle<RenderPassCommandRecorder_t> &renderPassCommandRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<RenderPassCommandRecorder_t> m_renderPassCommandRecorder;

    friend class CommandRecorder;
};

} // namespace ToyRenderer
