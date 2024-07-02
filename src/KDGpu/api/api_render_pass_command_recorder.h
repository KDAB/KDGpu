/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct GraphicsPipeline_t;
struct PipelineLayout_t;
struct TextureView_t;
struct DrawCommand;
struct DrawIndexedCommand;
struct DrawIndirectCommand;
struct DrawIndexedIndirectCommand;
struct DrawMeshCommand;
struct DrawMeshIndirectCommand;
struct PushConstantRange;

/**
 * @brief ApiRenderPassCommandRecorder
 * \ingroup api
 *
 */
struct ApiRenderPassCommandRecorder {
    virtual void setPipeline(const Handle<GraphicsPipeline_t> &pipeline) = 0;
    virtual void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset) = 0;
    virtual void setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType) = 0;
    virtual void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                              const Handle<PipelineLayout_t> &pipelineLayout, const std::vector<uint32_t> &dynamicBufferOffsets) = 0;
    virtual void setViewport(const Viewport &viewport) = 0;
    virtual void setScissor(const Rect2D &scissor) = 0;
    virtual void setStencilReference(StencilFaceFlags faceMask, int reference) = 0;
    virtual void draw(const DrawCommand &drawCommand) = 0;
    virtual void draw(const std::vector<DrawCommand> &drawCommands) = 0;
    virtual void drawIndexed(const DrawIndexedCommand &drawCommand) = 0;
    virtual void drawIndexed(const std::vector<DrawIndexedCommand> &drawCommands) = 0;
    virtual void drawIndirect(const DrawIndirectCommand &drawCommand) = 0;
    virtual void drawIndirect(const std::vector<DrawIndirectCommand> &drawCommands) = 0;
    virtual void drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand) = 0;
    virtual void drawIndexedIndirect(const std::vector<DrawIndexedIndirectCommand> &drawCommands) = 0;
    virtual void drawMeshTasks(const DrawMeshCommand &drawCommand) = 0;
    virtual void drawMeshTasks(const std::vector<DrawMeshCommand> &drawCommands) = 0;
    virtual void drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand) = 0;
    virtual void drawMeshTasksIndirect(const std::vector<DrawMeshIndirectCommand> &drawCommands) = 0;
    virtual void pushConstant(const PushConstantRange &constantRange, const void *data) = 0;
    virtual void end() = 0;
};

} // namespace KDGpu
