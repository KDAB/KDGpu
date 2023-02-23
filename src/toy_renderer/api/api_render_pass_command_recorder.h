#pragma once

#include <toy_renderer/handle.h>

namespace ToyRenderer {

struct Buffer_t;
struct GraphicsPipeline_t;

struct DrawCommand;

struct ApiRenderPassCommandRecorder {
    virtual void setPipeline(const Handle<GraphicsPipeline_t> &pipeline) = 0;
    virtual void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer) = 0;
    virtual void draw(const DrawCommand &drawCommand) = 0;
    virtual void end() = 0;
};

} // namespace ToyRenderer
