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
}

void RenderPassCommandRecorder::setPipeline(const Handle<GraphicsPipeline_t> &pipeline)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setPipeline(pipeline);
}

void RenderPassCommandRecorder::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setVertexBuffer(index, buffer);
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
    // TODO: Implement me!
}

} // namespace ToyRenderer
