/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

// WARNING: Only include Vulkan-implementation headers in this file.
// Do not include this from public headers.

#include "render_pass_command_recorder_base.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

void RenderPassCommandRecorderBase::setPipeline(const RequiredHandle<GraphicsPipeline_t> &pipeline)
{
    backend()->setPipeline(pipeline);
}

void RenderPassCommandRecorderBase::setVertexBuffer(uint32_t index, const RequiredHandle<Buffer_t> &buffer, DeviceSize offset)
{
    backend()->setVertexBuffer(index, buffer, offset);
}

void RenderPassCommandRecorderBase::setIndexBuffer(const RequiredHandle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType)
{
    backend()->setIndexBuffer(buffer, offset, indexType);
}

void RenderPassCommandRecorderBase::setBindGroup(uint32_t group,
                                                 const RequiredHandle<BindGroup_t> &bindGroup,
                                                 const OptionalHandle<PipelineLayout_t> &pipelineLayout,
                                                 std::span<const uint32_t> dynamicBufferOffsets)
{
    backend()->setBindGroup(group, bindGroup, pipelineLayout, dynamicBufferOffsets);
}

void RenderPassCommandRecorderBase::setViewport(const Viewport &viewport)
{
    backend()->setViewport(viewport);
}

void RenderPassCommandRecorderBase::setScissor(const Rect2D &scissor)
{
    backend()->setScissor(scissor);
}

void RenderPassCommandRecorderBase::setStencilReference(StencilFaceFlags faceMask, int reference)
{
    backend()->setStencilReference(faceMask, reference);
}

void RenderPassCommandRecorderBase::draw(const DrawCommand &drawCommand)
{
    backend()->draw(drawCommand);
}

void RenderPassCommandRecorderBase::draw(std::span<const DrawCommand> drawCommands)
{
    backend()->draw(drawCommands);
}

void RenderPassCommandRecorderBase::drawIndexed(const DrawIndexedCommand &drawCommand)
{
    backend()->drawIndexed(drawCommand);
}

void RenderPassCommandRecorderBase::drawIndexed(std::span<const DrawIndexedCommand> drawCommands)
{
    backend()->drawIndexed(drawCommands);
}

void RenderPassCommandRecorderBase::drawIndirect(const DrawIndirectCommand &drawCommand)
{
    backend()->drawIndirect(drawCommand);
}

void RenderPassCommandRecorderBase::drawIndirect(std::span<const DrawIndirectCommand> drawCommands)
{
    backend()->drawIndirect(drawCommands);
}

void RenderPassCommandRecorderBase::drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand)
{
    backend()->drawIndexedIndirect(drawCommand);
}

void RenderPassCommandRecorderBase::drawIndexedIndirect(std::span<const DrawIndexedIndirectCommand> drawCommands)
{
    backend()->drawIndexedIndirect(drawCommands);
}

void RenderPassCommandRecorderBase::drawMeshTasks(const DrawMeshCommand &drawCommand)
{
    backend()->drawMeshTasks(drawCommand);
}

void RenderPassCommandRecorderBase::drawMeshTasks(std::span<const DrawMeshCommand> drawCommands)
{
    backend()->drawMeshTasks(drawCommands);
}

void RenderPassCommandRecorderBase::drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand)
{
    backend()->drawMeshTasksIndirect(drawCommand);
}

void RenderPassCommandRecorderBase::drawMeshTasksIndirect(std::span<const DrawMeshIndirectCommand> drawCommands)
{
    backend()->drawMeshTasksIndirect(drawCommands);
}

void RenderPassCommandRecorderBase::pushConstant(const PushConstantRange &constantRange, const void *data,
                                                 const OptionalHandle<PipelineLayout_t> &pipelineLayout)
{
    backend()->pushConstant(constantRange, data, pipelineLayout);
}

void RenderPassCommandRecorderBase::pushBindGroup(uint32_t group,
                                                  std::span<const BindGroupEntry> bindGroupEntries,
                                                  const Handle<PipelineLayout_t> &pipelineLayout)
{
    backend()->pushBindGroup(group, bindGroupEntries, pipelineLayout);
}

void RenderPassCommandRecorderBase::end()
{
    backend()->end();
}

} // namespace KDGpu
