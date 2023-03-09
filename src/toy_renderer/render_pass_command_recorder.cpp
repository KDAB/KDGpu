#include "render_pass_command_recorder.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_render_pass_command_recorder.h>

namespace ToyRenderer {

RenderPassCommandRecorder::RenderPassCommandRecorder(GraphicsApi *api,
                                                     const Handle<Device_t> &device,
                                                     const Handle<RenderPassCommandRecorder_t> &renderPassCommandRecorder)
    : m_api(api)
    , m_device(device)
    , m_renderPassCommandRecorder(renderPassCommandRecorder)
{
}

RenderPassCommandRecorder::~RenderPassCommandRecorder()
{
    if (isValid())
        m_api->resourceManager()->deleteRenderPassCommandRecorder(handle());
}

RenderPassCommandRecorder::RenderPassCommandRecorder(RenderPassCommandRecorder &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_renderPassCommandRecorder = other.m_renderPassCommandRecorder;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_renderPassCommandRecorder = {};
}

RenderPassCommandRecorder &RenderPassCommandRecorder::operator=(RenderPassCommandRecorder &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteRenderPassCommandRecorder(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_renderPassCommandRecorder = other.m_renderPassCommandRecorder;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_renderPassCommandRecorder = {};
    }
    return *this;
}

void RenderPassCommandRecorder::setPipeline(const Handle<GraphicsPipeline_t> &pipeline)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setPipeline(pipeline);
}

void RenderPassCommandRecorder::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setVertexBuffer(index, buffer, offset);
}

void RenderPassCommandRecorder::setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setIndexBuffer(buffer, offset, indexType);
}

void RenderPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup, const Handle<PipelineLayout_t> &pipelineLayout)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setBindGroup(group, bindGroup, pipelineLayout);
}

void RenderPassCommandRecorder::setViewport(const Viewport &viewport)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setViewport(viewport);
}

void RenderPassCommandRecorder::setScissor(const Rect2D &scissor)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setScissor(scissor);
}

void RenderPassCommandRecorder::end()
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->end();
}

void RenderPassCommandRecorder::draw(const DrawCommand &drawCommand)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->draw(drawCommand);
}

void RenderPassCommandRecorder::draw(const std::vector<DrawCommand> &drawCommands)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->draw(drawCommands);
}

void RenderPassCommandRecorder::drawIndexed(const DrawIndexedCommand &drawCommand)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawIndexed(drawCommand);
}

void RenderPassCommandRecorder::drawIndexed(const std::vector<DrawIndexedCommand> &drawCommands)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawIndexed(drawCommands);
}

void RenderPassCommandRecorder::drawIndirect(const DrawIndirectCommand &drawCommand)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawIndirect(drawCommand);
}

void RenderPassCommandRecorder::drawIndirect(const std::vector<DrawIndirectCommand> &drawCommands)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawIndirect(drawCommands);
}

void RenderPassCommandRecorder::drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawIndexedIndirect(drawCommand);
}

void RenderPassCommandRecorder::drawIndexedIndirect(const std::vector<DrawIndexedIndirectCommand> &drawCommands)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawIndexedIndirect(drawCommands);
}

void RenderPassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->pushConstant(constantRange, data);
}

} // namespace ToyRenderer
