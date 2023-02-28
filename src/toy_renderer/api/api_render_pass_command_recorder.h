#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>

namespace ToyRenderer {

struct BindGroup_t;
struct Buffer_t;
struct GraphicsPipeline_t;
struct TextureView_t;
struct DrawCommand;
struct DrawIndexedCommand;
struct PushConstantRange;

struct ApiRenderPassCommandRecorder {
    virtual void setPipeline(const Handle<GraphicsPipeline_t> &pipeline) = 0;
    virtual void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer) = 0;
    virtual void setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType) = 0;
    virtual void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup) = 0;
    virtual void setViewport(const Viewport &viewport) = 0;
    virtual void setScissor(const Rect2D &scissor) = 0;
    virtual void draw(const DrawCommand &drawCommand) = 0;
    virtual void draw(const std::vector<DrawCommand> &drawCommands) = 0;
    virtual void drawIndexed(const DrawIndexedCommand &drawCommand) = 0;
    virtual void drawIndexed(const std::vector<DrawIndexedCommand> &drawCommands) = 0;
    virtual void pushConstant(const PushConstantRange &constantRange, const std::vector<uint8_t> &data) = 0;
    virtual void end() = 0;
};

} // namespace ToyRenderer
