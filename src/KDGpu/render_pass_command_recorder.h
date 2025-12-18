/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

#include <span>
#include <optional>

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct GraphicsPipeline_t;
struct PipelineLayout_t;
struct RenderPassCommandRecorder_t;

struct Rect2D;
struct Viewport;
struct PushConstantRange;
struct BindGroupEntry;

struct DrawCommand {
    uint32_t vertexCount{ 0 };
    uint32_t instanceCount{ 1 };
    uint32_t firstVertex{ 0 };
    uint32_t firstInstance{ 0 };
};

struct DrawIndexedCommand {
    uint32_t indexCount{ 0 };
    uint32_t instanceCount{ 1 };
    uint32_t firstIndex{ 0 };
    int32_t vertexOffset{ 0 };
    uint32_t firstInstance{ 0 };
};

struct DrawIndirectCommand {
    Handle<Buffer_t> buffer;
    size_t offset{ 0 };
    uint32_t drawCount{ 0 };
    uint32_t stride{ 0 };
};

struct DrawIndexedIndirectCommand {
    Handle<Buffer_t> buffer;
    size_t offset{ 0 };
    uint32_t drawCount{ 0 };
    uint32_t stride{ 0 };
};

struct DrawMeshCommand {
    uint32_t workGroupX{ 1 };
    uint32_t workGroupY{ 1 };
    uint32_t workGroupZ{ 1 };
};

struct DrawMeshIndirectCommand {
    Handle<Buffer_t> buffer;
    size_t offset{ 0 };
    uint32_t drawCount{ 0 };
    uint32_t stride{ 0 };
};

/**
 * @brief RenderPassCommandRecorder
 * @ingroup public
 */
class KDGPU_EXPORT RenderPassCommandRecorder
{
public:
    ~RenderPassCommandRecorder();

    RenderPassCommandRecorder(RenderPassCommandRecorder &&) noexcept;
    RenderPassCommandRecorder &operator=(RenderPassCommandRecorder &&) noexcept;

    RenderPassCommandRecorder(const RenderPassCommandRecorder &) = delete;
    RenderPassCommandRecorder &operator=(const RenderPassCommandRecorder &) = delete;

    const Handle<RenderPassCommandRecorder_t> &handle() const noexcept { return m_renderPassCommandRecorder; }
    bool isValid() const noexcept { return m_renderPassCommandRecorder.isValid(); }

    operator Handle<RenderPassCommandRecorder_t>() const noexcept { return m_renderPassCommandRecorder; }

    void setPipeline(const Handle<GraphicsPipeline_t> &pipeline);

    // TODO: Add overload for setting many vertex buffers at once
    void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset = 0);
    void setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset = 0, IndexType indexType = IndexType::Uint32);

    void setBindGroup(uint32_t group,
                      const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>(),
                      std::span<const uint32_t> dynamicBufferOffsets = {});

    void setViewport(const Viewport &viewport);
    void setScissor(const Rect2D &scissor);
    void setStencilReference(StencilFaceFlags faceMask, int reference);

    void draw(const DrawCommand &drawCommand);
    void draw(std::span<const DrawCommand> drawCommands);

    void drawIndexed(const DrawIndexedCommand &drawCommand);
    void drawIndexed(std::span<const DrawIndexedCommand> drawCommands);

    void drawIndirect(const DrawIndirectCommand &drawCommand);
    void drawIndirect(std::span<const DrawIndirectCommand> drawCommands);

    void drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand);
    void drawIndexedIndirect(std::span<const DrawIndexedIndirectCommand> drawCommands);

    void drawMeshTasks(const DrawMeshCommand &drawCommand);
    void drawMeshTasks(std::span<const DrawMeshCommand> drawCommands);

    void drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand);
    void drawMeshTasksIndirect(std::span<const DrawMeshIndirectCommand> drawCommands);

    void pushConstant(const PushConstantRange &constantRange, const void *data, const Handle<PipelineLayout_t> &pipelineLayout = {});
    void pushBindGroup(uint32_t group,
                       std::span<const BindGroupEntry> bindGroupEntries,
                       const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>());

    void nextSubpass();

    // Remap Dynamic Rendering attachments to input attachments for the following draw calls
    // (e.g RenderTarget Color[0] -> Input[2])
    void setInputAttachmentMapping(std::span<const uint32_t> colorAttachmentIndices,
                                   std::optional<uint32_t> depthAttachmentIndex,
                                   std::optional<uint32_t> stencilAttachmentIndex);

    // Remap Fragment Outputs for the following draw calls [e.g RenderTarget Color[0] -> Output[2]]
    void setOutputAttachmentMapping(std::span<const uint32_t> remappedOutputs);

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

} // namespace KDGpu
