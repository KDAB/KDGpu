/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group.h>
#include <KDGpu/buffer.h>
#include <KDGpu/graphics_pipeline.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/render_pass_command_recorder.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanRenderPassCommandRecorder
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanRenderPassCommandRecorder {
    explicit VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                             VkRect2D _renderArea,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle,
                                             bool _dynamicRendering);

    void setPipeline(const Handle<GraphicsPipeline_t> &pipeline);
    void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset) const;
    void setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType) const;
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout, std::span<const uint32_t> dynamicBufferOffsets) const;
    void setViewport(const Viewport &viewport) const;
    void setScissor(const Rect2D &scissor) const;
    void setStencilReference(StencilFaceFlags faceMask, int reference) const;
    void draw(const DrawCommand &drawCommand) const;
    void draw(std::span<const DrawCommand> drawCommands) const;
    void drawIndexed(const DrawIndexedCommand &drawCommand) const;
    void drawIndexed(std::span<const DrawIndexedCommand> drawCommands) const;
    void drawIndirect(const DrawIndirectCommand &drawCommand) const;
    void drawIndirect(std::span<const DrawIndirectCommand> drawCommands) const;
    void drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand) const;
    void drawIndexedIndirect(std::span<const DrawIndexedIndirectCommand> drawCommands) const;
    void drawMeshTasks(const DrawMeshCommand &drawCommand) const;
    void drawMeshTasks(std::span<const DrawMeshCommand> drawCommands) const;
    void drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand) const;
    void drawMeshTasksIndirect(std::span<const DrawMeshIndirectCommand> drawCommands) const;
    void pushConstant(const PushConstantRange &constantRange, const void *data, const Handle<PipelineLayout_t> &pipelineLayout = {}) const;
    void pushBindGroup(uint32_t group, std::span<const BindGroupEntry> bindGroupEntries, const Handle<PipelineLayout_t> &pipelineLayout = {}) const;
    void nextSubpass() const;
    void setInputAttachmentMapping(std::span<const uint32_t> colorAttachmentIndices,
                                   std::optional<uint32_t> depthAttachmentIndex,
                                   std::optional<uint32_t> stencilAttachmentIndex) const;
    void setOutputAttachmentMapping(std::span<const uint32_t> remappedOutputs) const;
    void end() const;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkRect2D renderArea{};
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<GraphicsPipeline_t> pipeline;
    bool firstPipelineWasSet{ false };
    bool dynamicRendering{ false };
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace KDGpu
