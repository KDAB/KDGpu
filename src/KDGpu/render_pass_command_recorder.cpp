/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "render_pass_command_recorder.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

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

RenderPassCommandRecorder::RenderPassCommandRecorder(RenderPassCommandRecorder &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_renderPassCommandRecorder = std::exchange(other.m_renderPassCommandRecorder, {});
}

RenderPassCommandRecorder &RenderPassCommandRecorder::operator=(RenderPassCommandRecorder &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteRenderPassCommandRecorder(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_renderPassCommandRecorder = std::exchange(other.m_renderPassCommandRecorder, {});
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

void RenderPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                                             const Handle<PipelineLayout_t> &pipelineLayout,
                                             const std::vector<uint32_t> &dynamicBufferOffsets)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setBindGroup(group, bindGroup, pipelineLayout, dynamicBufferOffsets);
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

void RenderPassCommandRecorder::setStencilReference(const StencilFaceFlags faceMask, const int reference)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setStencilReference(faceMask, reference);
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

void RenderPassCommandRecorder::drawMeshTasks(const DrawMeshCommand &drawCommand)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawMeshTasks(drawCommand);
}

void RenderPassCommandRecorder::drawMeshTasks(const std::vector<DrawMeshCommand> &drawCommands)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawMeshTasks(drawCommands);
}

void RenderPassCommandRecorder::drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawMeshTasksIndirect(drawCommand);
}

void RenderPassCommandRecorder::drawMeshTasksIndirect(const std::vector<DrawMeshIndirectCommand> &drawCommands)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->drawMeshTasksIndirect(drawCommands);
}

void RenderPassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data, const Handle<PipelineLayout_t> &pipelineLayout)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->pushConstant(constantRange, data, pipelineLayout);
}

void RenderPassCommandRecorder::pushBindGroup(uint32_t group,
                                              const std::vector<BindGroupEntry> &bindGroupEntries,
                                              const Handle<PipelineLayout_t> &pipelineLayout)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->pushBindGroup(group, bindGroupEntries, pipelineLayout);
}

void RenderPassCommandRecorder::nextSubpass()
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->nextSubpass();
}

void RenderPassCommandRecorder::setInputAttachmentMapping(std::vector<uint32_t> colorAttachmentIndices,
                                                          std::optional<uint32_t> depthAttachmentIndex,
                                                          std::optional<uint32_t> stencilAttachmentIndex)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setInputAttachmentMapping(colorAttachmentIndices, depthAttachmentIndex, stencilAttachmentIndex);
}

void RenderPassCommandRecorder::setOutputAttachmentMapping(std::vector<uint32_t> remappedOutputs)
{
    auto apiRenderPassCommandRecorder = m_api->resourceManager()->getRenderPassCommandRecorder(m_renderPassCommandRecorder);
    apiRenderPassCommandRecorder->setOutputAttachmentMapping(remappedOutputs);
}

} // namespace KDGpu
