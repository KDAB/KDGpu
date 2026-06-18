/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_render_pass_command_recorder_base.h"

#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_graphics_pipeline.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <array>
#include <cassert>

namespace KDGpu {

VulkanRenderPassCommandRecorderBase::VulkanRenderPassCommandRecorderBase(VkCommandBuffer _commandBuffer,
                                                                         VkRect2D _renderArea,
                                                                         VulkanResourceManager *_vulkanResourceManager,
                                                                         const Handle<Device_t> &_deviceHandle)
    : commandBuffer(_commandBuffer)
    , renderArea(_renderArea)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanRenderPassCommandRecorderBase::setPipeline(const Handle<GraphicsPipeline_t> &_pipeline)
{
    pipeline = _pipeline;
    VulkanGraphicsPipeline *vulkanGraphicsPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsPipeline->pipeline);

    if (!firstPipelineWasSet) {
        // Set the initial viewport and scissor rect to the full extent of the render area
        VkViewport vkViewport = {
            .x = static_cast<float>(renderArea.offset.x),
            .y = static_cast<float>(renderArea.offset.y),
            .width = static_cast<float>(renderArea.extent.width),
            .height = static_cast<float>(renderArea.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &renderArea);

        firstPipelineWasSet = true;
    }
}

void VulkanRenderPassCommandRecorderBase::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    const std::array<VkBuffer, 1> buffers = { vulkanBuffer->buffer };
    const std::array<VkDeviceSize, 1> offsets = { offset };

    vkCmdBindVertexBuffers(commandBuffer, index, 1, buffers.data(), offsets.data());
}

void VulkanRenderPassCommandRecorderBase::setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    vkCmdBindIndexBuffer(commandBuffer, vulkanBuffer->buffer, offset, indexTypeToVkIndexType(indexType));
}

void VulkanRenderPassCommandRecorderBase::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroupH,
                                                       const Handle<PipelineLayout_t> &pipelineLayout,
                                                       std::span<const uint32_t> dynamicBufferOffsets) const
{
    VulkanBindGroup *bindGroup = vulkanResourceManager->getBindGroup(bindGroupH);
    VkDescriptorSet set = bindGroup->descriptorSet;

    // Use the pipeline layout provided, otherwise fallback to the one from the currently
    // bound pipeline (if any).
    VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
    if (pipelineLayout.isValid()) {
        VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(pipelineLayout);
        if (vulkanPipelineLayout)
            vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
    } else if (pipeline.isValid()) {
        VulkanGraphicsPipeline *vulkanPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);
        if (vulkanPipeline) {
            VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);
            if (vulkanPipelineLayout)
                vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
        }
    }

    assert(vkPipelineLayout != VK_NULL_HANDLE); // The PipelineLayout should outlive the pipelines

    // Bind Descriptor Set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            vkPipelineLayout,
                            group,
                            1, &set,
                            dynamicBufferOffsets.size(), dynamicBufferOffsets.data());
}

void VulkanRenderPassCommandRecorderBase::setViewport(const Viewport &viewport) const
{
    VkViewport vkViewport = {
        .x = viewport.x,
        .y = viewport.y,
        .width = viewport.width,
        .height = viewport.height,
        .minDepth = viewport.minDepth,
        .maxDepth = viewport.maxDepth
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
}

void VulkanRenderPassCommandRecorderBase::setScissor(const Rect2D &scissor) const
{
    VkRect2D vkScissor = {
        .offset = { .x = scissor.offset.x, .y = scissor.offset.y },
        .extent = { .width = scissor.extent.width, .height = scissor.extent.height }
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
}

void VulkanRenderPassCommandRecorderBase::setStencilReference(const StencilFaceFlags faceMask, const int reference) const
{
    vkCmdSetStencilReference(commandBuffer, stencilFaceToVkStencilFace(faceMask), reference);
}

void VulkanRenderPassCommandRecorderBase::draw(const DrawCommand &drawCommand) const
{
    vkCmdDraw(commandBuffer,
              drawCommand.vertexCount,
              drawCommand.instanceCount,
              drawCommand.firstVertex,
              drawCommand.firstInstance);
}

void VulkanRenderPassCommandRecorderBase::draw(std::span<const DrawCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        draw(drawCommand);
}

void VulkanRenderPassCommandRecorderBase::drawIndexed(const DrawIndexedCommand &drawCommand) const
{
    vkCmdDrawIndexed(commandBuffer,
                     drawCommand.indexCount,
                     drawCommand.instanceCount,
                     drawCommand.firstIndex,
                     drawCommand.vertexOffset,
                     drawCommand.firstInstance);
}

void VulkanRenderPassCommandRecorderBase::drawIndexed(std::span<const DrawIndexedCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawIndexed(drawCommand);
}

void VulkanRenderPassCommandRecorderBase::drawIndirect(const DrawIndirectCommand &drawCommand) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
    vkCmdDrawIndirect(commandBuffer,
                      vulkanBuffer->buffer,
                      drawCommand.offset,
                      drawCommand.drawCount,
                      drawCommand.stride);
}

void VulkanRenderPassCommandRecorderBase::drawIndirect(std::span<const DrawIndirectCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorderBase::drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
    vkCmdDrawIndexedIndirect(commandBuffer,
                             vulkanBuffer->buffer,
                             drawCommand.offset,
                             drawCommand.drawCount,
                             drawCommand.stride);
}

void VulkanRenderPassCommandRecorderBase::drawIndexedIndirect(std::span<const DrawIndexedIndirectCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawIndexedIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorderBase::drawMeshTasks(const DrawMeshCommand &drawCommand) const
{
#if VK_EXT_mesh_shader
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdDrawMeshTasksEXT) {
        device->vkCmdDrawMeshTasksEXT(commandBuffer,
                                      drawCommand.workGroupX,
                                      drawCommand.workGroupY,
                                      drawCommand.workGroupZ);
    }
#else
    assert(false);
#endif
}

void VulkanRenderPassCommandRecorderBase::drawMeshTasks(std::span<const DrawMeshCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawMeshTasks(drawCommand);
}

void VulkanRenderPassCommandRecorderBase::drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand) const
{
#if VK_EXT_mesh_shader
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdDrawMeshTasksIndirectEXT) {
        VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
        device->vkCmdDrawMeshTasksIndirectEXT(commandBuffer,
                                              vulkanBuffer->buffer,
                                              drawCommand.offset,
                                              drawCommand.drawCount,
                                              drawCommand.stride);
    }
#else
    assert(false);
#endif
}

void VulkanRenderPassCommandRecorderBase::drawMeshTasksIndirect(std::span<const DrawMeshIndirectCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawMeshTasksIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorderBase::pushConstant(const PushConstantRange &constantRange,
                                                       const void *data,
                                                       const Handle<PipelineLayout_t> &pipelineLayout) const
{
    VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };

    if (pipelineLayout.isValid()) {
        VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(pipelineLayout);
        if (vulkanPipelineLayout)
            vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
    } else if (pipeline.isValid()) {
        VulkanGraphicsPipeline *vulkanPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);
        VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);
        if (vulkanPipelineLayout)
            vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
    }

    vkCmdPushConstants(commandBuffer,
                       vkPipelineLayout,
                       constantRange.shaderStages.toInt(),
                       constantRange.offset,
                       constantRange.size,
                       data);
}

void VulkanRenderPassCommandRecorderBase::pushBindGroup(uint32_t group,
                                                        std::span<const BindGroupEntry> bindGroupEntries,
                                                        const Handle<PipelineLayout_t> &pipelineLayout) const
{
#if VK_KHR_push_descriptor
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdPushDescriptorSetKHR) {

        VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };

        if (pipelineLayout.isValid()) {
            VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(pipelineLayout);
            if (vulkanPipelineLayout)
                vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
        } else if (pipeline.isValid()) {
            VulkanGraphicsPipeline *vulkanPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);
            VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);
            if (vulkanPipelineLayout)
                vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
        }

        std::vector<WriteBindGroupData> writeBindGroupData;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;
        const size_t bindGroupEntryCount = bindGroupEntries.size();
        writeBindGroupData.resize(bindGroupEntryCount);
        writeDescriptorSets.resize(bindGroupEntryCount);
        for (size_t i = 0; i < bindGroupEntryCount; ++i) {
            WriteBindGroupData &writeData = writeBindGroupData[i];
            device->fillWriteBindGroupDataForBindGroupEntry(writeData, bindGroupEntries[i]);
            writeDescriptorSets[i] = writeData.descriptorWrite;
        }

        device->vkCmdPushDescriptorSetKHR(commandBuffer,
                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          vkPipelineLayout,
                                          group,
                                          writeDescriptorSets.size(),
                                          writeDescriptorSets.data());
    }
#else
    assert(false);
#endif
}

} // namespace KDGpu
