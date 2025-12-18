/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_render_pass_command_recorder.h"

#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_graphics_pipeline.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <array>

namespace KDGpu {

VulkanRenderPassCommandRecorder::VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                 VkRect2D _renderArea,
                                                                 VulkanResourceManager *_vulkanResourceManager,
                                                                 const Handle<Device_t> &_deviceHandle,
                                                                 bool _dynamicRendering)
    : commandBuffer(_commandBuffer)
    , renderArea(_renderArea)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , dynamicRendering(_dynamicRendering)
{
}

void VulkanRenderPassCommandRecorder::setPipeline(const Handle<GraphicsPipeline_t> &_pipeline)
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

void VulkanRenderPassCommandRecorder::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    const std::array<VkBuffer, 1> buffers = { vulkanBuffer->buffer };
    const std::array<VkDeviceSize, 1> offsets = { offset };

    vkCmdBindVertexBuffers(commandBuffer, index, 1, buffers.data(), offsets.data());
}

void VulkanRenderPassCommandRecorder::setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    vkCmdBindIndexBuffer(commandBuffer, vulkanBuffer->buffer, offset, indexTypeToVkIndexType(indexType));
}

void VulkanRenderPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroupH,
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

void VulkanRenderPassCommandRecorder::setViewport(const Viewport &viewport) const
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

void VulkanRenderPassCommandRecorder::setScissor(const Rect2D &scissor) const
{
    VkRect2D vkScissor = {
        .offset = { .x = scissor.offset.x, .y = scissor.offset.y },
        .extent = { .width = scissor.extent.width, .height = scissor.extent.height }
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
}

void VulkanRenderPassCommandRecorder::setStencilReference(const StencilFaceFlags faceMask, const int reference) const
{
    vkCmdSetStencilReference(commandBuffer, stencilFaceToVkStencilFace(faceMask), reference);
}

void VulkanRenderPassCommandRecorder::draw(const DrawCommand &drawCommand) const
{
    vkCmdDraw(commandBuffer,
              drawCommand.vertexCount,
              drawCommand.instanceCount,
              drawCommand.firstVertex,
              drawCommand.firstInstance);
}

void VulkanRenderPassCommandRecorder::draw(std::span<const DrawCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        draw(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawIndexed(const DrawIndexedCommand &drawCommand) const
{
    vkCmdDrawIndexed(commandBuffer,
                     drawCommand.indexCount,
                     drawCommand.instanceCount,
                     drawCommand.firstIndex,
                     drawCommand.vertexOffset,
                     drawCommand.firstInstance);
}

void VulkanRenderPassCommandRecorder::drawIndexed(std::span<const DrawIndexedCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawIndexed(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawIndirect(const DrawIndirectCommand &drawCommand) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
    vkCmdDrawIndirect(commandBuffer,
                      vulkanBuffer->buffer,
                      drawCommand.offset,
                      drawCommand.drawCount,
                      drawCommand.stride);
}

void VulkanRenderPassCommandRecorder::drawIndirect(std::span<const DrawIndirectCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand) const
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
    vkCmdDrawIndexedIndirect(commandBuffer,
                             vulkanBuffer->buffer,
                             drawCommand.offset,
                             drawCommand.drawCount,
                             drawCommand.stride);
}

void VulkanRenderPassCommandRecorder::drawIndexedIndirect(std::span<const DrawIndexedIndirectCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawIndexedIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawMeshTasks(const DrawMeshCommand &drawCommand) const
{
#if defined(VK_EXT_mesh_shader)
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

void VulkanRenderPassCommandRecorder::drawMeshTasks(std::span<const DrawMeshCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawMeshTasks(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand) const
{
#if defined(VK_EXT_mesh_shader)
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

void VulkanRenderPassCommandRecorder::drawMeshTasksIndirect(std::span<const DrawMeshIndirectCommand> drawCommands) const
{
    for (const auto &drawCommand : drawCommands)
        drawMeshTasksIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorder::pushConstant(const PushConstantRange &constantRange,
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

void VulkanRenderPassCommandRecorder::pushBindGroup(uint32_t group,
                                                    std::span<const BindGroupEntry> bindGroupEntries,
                                                    const Handle<PipelineLayout_t> &pipelineLayout) const
{
#if defined(VK_KHR_push_descriptor)
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

void VulkanRenderPassCommandRecorder::nextSubpass() const
{
    if (!dynamicRendering) {
        // For now we assume renderpass/subpass are always recorded inline (primary command buffer)
        vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
    }
}

void VulkanRenderPassCommandRecorder::setOutputAttachmentMapping(std::span<const uint32_t> remappedOutputs) const
{
#if defined(VK_KHR_dynamic_rendering_local_read)
    assert(dynamicRendering);
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdSetRenderingAttachmentLocationsKHR) {
        VkRenderingAttachmentLocationInfoKHR locationInfo{};
        locationInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        locationInfo.colorAttachmentCount = remappedOutputs.size();
        locationInfo.pColorAttachmentLocations = remappedOutputs.data();
        device->vkCmdSetRenderingAttachmentLocationsKHR(commandBuffer, &locationInfo);
    }
#else
    assert(false);
#endif
}

void VulkanRenderPassCommandRecorder::setInputAttachmentMapping(std::span<const uint32_t> colorAttachmentIndices,
                                                                std::optional<uint32_t> depthAttachmentIndex,
                                                                std::optional<uint32_t> stencilAttachmentIndex) const
{
#if defined(VK_KHR_dynamic_rendering_local_read)
    assert(dynamicRendering);
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdSetRenderingInputAttachmentIndicesKHR) {
        const uint32_t depthInputLocationIdx = depthAttachmentIndex ? *depthAttachmentIndex : VK_ATTACHMENT_UNUSED;
        const uint32_t stencilInputLocationIdx = stencilAttachmentIndex ? *stencilAttachmentIndex : VK_ATTACHMENT_UNUSED;

        VkRenderingInputAttachmentIndexInfoKHR locationInfo{};
        locationInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR;
        locationInfo.colorAttachmentCount = colorAttachmentIndices.size();
        locationInfo.pColorAttachmentInputIndices = colorAttachmentIndices.data();
        locationInfo.pDepthInputAttachmentIndex = &depthInputLocationIdx;
        locationInfo.pStencilInputAttachmentIndex = &stencilInputLocationIdx;

        device->vkCmdSetRenderingInputAttachmentIndicesKHR(commandBuffer, &locationInfo);
    }
#else
    assert(false);
#endif
}

void VulkanRenderPassCommandRecorder::end() const
{
    if (dynamicRendering) {
#if defined(VK_KHR_dynamic_rendering)
        VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
        device->vkCmdEndRenderingKHR(commandBuffer);
#endif
    } else {
        vkCmdEndRenderPass(commandBuffer);
    }
}

} // namespace KDGpu
