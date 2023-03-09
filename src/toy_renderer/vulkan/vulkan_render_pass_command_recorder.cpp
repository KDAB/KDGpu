#include "vulkan_render_pass_command_recorder.h"

#include <toy_renderer/vulkan/vulkan_enums.h>
#include <toy_renderer/vulkan/vulkan_graphics_pipeline.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <array>

namespace ToyRenderer {

VulkanRenderPassCommandRecorder::VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                 VkRect2D _renderArea,
                                                                 VulkanResourceManager *_vulkanResourceManager,
                                                                 const Handle<Device_t> &_deviceHandle)
    : ApiRenderPassCommandRecorder()
    , commandBuffer(_commandBuffer)
    , renderArea(_renderArea)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
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
}

void VulkanRenderPassCommandRecorder::setPipeline(const Handle<GraphicsPipeline_t> &_pipeline)
{
    pipeline = _pipeline;
    VulkanGraphicsPipeline *vulkanGraphicsPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsPipeline->pipeline);
}

void VulkanRenderPassCommandRecorder::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer, DeviceSize offset)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    const std::array<VkBuffer, 1> buffers = { vulkanBuffer->buffer };
    const std::array<VkDeviceSize, 1> offsets = { offset };

    vkCmdBindVertexBuffers(commandBuffer, index, 1, buffers.data(), offsets.data());
}

void VulkanRenderPassCommandRecorder::setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    vkCmdBindIndexBuffer(commandBuffer, vulkanBuffer->buffer, offset, indexTypeToVkIndexType(indexType));
}

void VulkanRenderPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroupH, const Handle<PipelineLayout_t> &pipelineLayout)
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
                            0, nullptr);
}

void VulkanRenderPassCommandRecorder::setViewport(const Viewport &viewport)
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

void VulkanRenderPassCommandRecorder::setScissor(const Rect2D &scissor)
{
    VkRect2D vkScissor = {
        .offset = { .x = scissor.offset.x, .y = scissor.offset.y },
        .extent = { .width = scissor.extent.width, .height = scissor.extent.height }
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
}

void VulkanRenderPassCommandRecorder::draw(const DrawCommand &drawCommand)
{
    vkCmdDraw(commandBuffer,
              drawCommand.vertexCount,
              drawCommand.instanceCount,
              drawCommand.firstVertex,
              drawCommand.firstInstance);
}

void VulkanRenderPassCommandRecorder::draw(const std::vector<DrawCommand> &drawCommands)
{
    for (const auto &drawCommand : drawCommands)
        draw(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawIndexed(const DrawIndexedCommand &drawCommand)
{
    vkCmdDrawIndexed(commandBuffer,
                     drawCommand.indexCount,
                     drawCommand.instanceCount,
                     drawCommand.firstIndex,
                     drawCommand.vertexOffset,
                     drawCommand.firstInstance);
}

void VulkanRenderPassCommandRecorder::drawIndexed(const std::vector<DrawIndexedCommand> &drawCommands)
{
    for (const auto &drawCommand : drawCommands)
        drawIndexed(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawIndirect(const DrawIndirectCommand &drawCommand)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
    vkCmdDrawIndirect(commandBuffer,
                      vulkanBuffer->buffer,
                      drawCommand.offset,
                      drawCommand.drawCount,
                      drawCommand.stride);
}

void VulkanRenderPassCommandRecorder::drawIndirect(const std::vector<DrawIndirectCommand> &drawCommands)
{
    for (const auto &drawCommand : drawCommands)
        drawIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorder::drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(drawCommand.buffer);
    vkCmdDrawIndexedIndirect(commandBuffer,
                             vulkanBuffer->buffer,
                             drawCommand.offset,
                             drawCommand.drawCount,
                             drawCommand.stride);
}

void VulkanRenderPassCommandRecorder::drawIndexedIndirect(const std::vector<DrawIndexedIndirectCommand> &drawCommands)
{
    for (const auto &drawCommand : drawCommands)
        drawIndexedIndirect(drawCommand);
}

void VulkanRenderPassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data)
{
    VulkanGraphicsPipeline *vulkanPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);
    VulkanPipelineLayout *pLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);

    assert(pLayout != nullptr); // The PipelineLayout should outlive the pipelines
    vkCmdPushConstants(commandBuffer,
                       pLayout->pipelineLayout,
                       shaderStageFlagBitsToVkShaderStageFlagBits(static_cast<ShaderStageFlagBits>(constantRange.shaderStages)),
                       constantRange.offset,
                       constantRange.size,
                       data);
}

void VulkanRenderPassCommandRecorder::end()
{
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace ToyRenderer
