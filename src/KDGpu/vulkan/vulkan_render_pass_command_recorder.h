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
    void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset);
    void setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType);
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout, const std::vector<uint32_t> &dynamicBufferOffsets);
    void setViewport(const Viewport &viewport);
    void setScissor(const Rect2D &scissor);
    void setStencilReference(StencilFaceFlags faceMask, int reference);
    void draw(const DrawCommand &drawCommand);
    void draw(const std::vector<DrawCommand> &drawCommands);
    void drawIndexed(const DrawIndexedCommand &drawCommand);
    void drawIndexed(const std::vector<DrawIndexedCommand> &drawCommands);
    void drawIndirect(const DrawIndirectCommand &drawCommand);
    void drawIndirect(const std::vector<DrawIndirectCommand> &drawCommands);
    void drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand);
    void drawIndexedIndirect(const std::vector<DrawIndexedIndirectCommand> &drawCommands);
    void drawMeshTasks(const DrawMeshCommand &drawCommand);
    void drawMeshTasks(const std::vector<DrawMeshCommand> &drawCommands);
    void drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand);
    void drawMeshTasksIndirect(const std::vector<DrawMeshIndirectCommand> &drawCommands);
    void pushConstant(const PushConstantRange &constantRange, const void *data, const Handle<PipelineLayout_t> &pipelineLayout = {});
    void pushBindGroup(uint32_t group, const std::vector<BindGroupEntry> &bindGroupEntries, const Handle<PipelineLayout_t> &pipelineLayout = {});
    void nextSubpass();
    void setInputAttachmentMapping(std::vector<uint32_t> colorAttachmentIndices,
                                   std::optional<uint32_t> depthAttachmentIndex,
                                   std::optional<uint32_t> stencilAttachmentIndex);
    void setOutputAttachmentMapping(std::vector<uint32_t> remappedOutputs);

    void end();

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkRect2D renderArea{};
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<GraphicsPipeline_t> pipeline;
    bool firstPipelineWasSet{ false };
    bool dynamicRendering{ false };
};

} // namespace KDGpu
