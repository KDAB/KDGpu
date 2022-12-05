#include "render_pass_command_recorder.h"

namespace ToyRenderer {

void RenderPassCommandRecorder::setPipeline(const Handle<GraphicsPipeline_t> &pipeline)
{
    // TODO: Implement me!
}

void RenderPassCommandRecorder::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer)
{
    // TODO: Implement me!
}

void RenderPassCommandRecorder::end()
{
    // TODO: Implement me!
}

void RenderPassCommandRecorder::draw(const DrawCommand &drawCmd)
{
    // TODO: Implement me!
}

void RenderPassCommandRecorder::draw(const std::vector<DrawCommand> &drawCmds)
{
    // TODO: Implement me!
}

} // namespace ToyRenderer
