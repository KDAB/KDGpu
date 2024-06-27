/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_render_pass_command_recorder.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanRenderPassCommandRecorder
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanRenderPassCommandRecorder : public ApiRenderPassCommandRecorder {
    explicit VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                             VkRect2D _renderArea,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle);

    void setPipeline(const Handle<GraphicsPipeline_t> &pipeline) final;
    void setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset) final;
    void setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType) final;
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout, const std::vector<uint32_t> &dynamicBufferOffsets) final;
    void setViewport(const Viewport &viewport) final;
    void setScissor(const Rect2D &scissor) final;
    void setStencilReference(StencilFaceFlags faceMask, int reference) final;
    void draw(const DrawCommand &drawCommand) final;
    void draw(const std::vector<DrawCommand> &drawCommands) final;
    void drawIndexed(const DrawIndexedCommand &drawCommand) final;
    void drawIndexed(const std::vector<DrawIndexedCommand> &drawCommands) final;
    void drawIndirect(const DrawIndirectCommand &drawCommand) final;
    void drawIndirect(const std::vector<DrawIndirectCommand> &drawCommands) final;
    void drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand) final;
    void drawIndexedIndirect(const std::vector<DrawIndexedIndirectCommand> &drawCommands) final;
    void drawMeshTasks(const DrawMeshCommand &drawCommand) final;
    void drawMeshTasks(const std::vector<DrawMeshCommand> &drawCommands) final;
    void drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand) final;
    void drawMeshTasksIndirect(const std::vector<DrawMeshIndirectCommand> &drawCommands) final;
    void pushConstant(const PushConstantRange &constantRange, const void *data) final;
    void nextSubpass() final;
    void end() final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkRect2D renderArea{};
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<GraphicsPipeline_t> pipeline;
    bool firstPipelineWasSet{ false };
};

} // namespace KDGpu
