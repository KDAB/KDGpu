#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/toy_renderer_export.h>

#include <vector>

namespace ToyRenderer {

struct Buffer_t;
struct GraphicsPipeline_t;

struct DrawCommand {
    uint32_t vertexCount{ 0 };
    uint32_t instanceCount{ 1 };
    uint32_t firstVertex{ 0 };
    uint32_t firstInstance{ 0 };
};

class TOY_RENDERER_EXPORT RenderPassCommandRecorder
{
public:
    void setPipeline(const Handle<GraphicsPipeline_t> &pipeline);
    void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer);

    void draw(const DrawCommand &drawCmd);
    void draw(const std::vector<DrawCommand> &drawCmds);

    void end();

private:
    friend class CommandRecorder;
};

} // namespace ToyRenderer
