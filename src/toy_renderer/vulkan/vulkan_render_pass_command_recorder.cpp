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

void VulkanRenderPassCommandRecorder::setVertexBuffer(uint32_t index, const Handle<Buffer_t> &buffer)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    std::array<VkBuffer, 1> buffers = { vulkanBuffer->buffer };
    std::array<VkDeviceSize, 1> offsets = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, index, 1, buffers.data(), offsets.data());
}

void VulkanRenderPassCommandRecorder::setIndexBuffer(const Handle<Buffer_t> &buffer, DeviceSize offset, IndexType indexType)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);
    vkCmdBindIndexBuffer(commandBuffer, vulkanBuffer->buffer, offset, indexTypeToVkIndexType(indexType));
}

void VulkanRenderPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroupH)
{
    VulkanBindGroup *bindGroup = vulkanResourceManager->getBindGroup(bindGroupH);
    VkDescriptorSet set = bindGroup->descriptorSet;

    // Bind Descriptor Set
    VulkanGraphicsPipeline *p = vulkanResourceManager->getGraphicsPipeline(pipeline);
    VkPipelineLayout pipelineLayout = p->pipelineLayout;
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
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
    for (const auto &drawCommand : drawCommands) {
        vkCmdDraw(commandBuffer,
                  drawCommand.vertexCount,
                  drawCommand.instanceCount,
                  drawCommand.firstVertex,
                  drawCommand.firstInstance);
    }
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
    for (const auto &drawCommand : drawCommands) {
        vkCmdDrawIndexed(commandBuffer,
                         drawCommand.indexCount,
                         drawCommand.instanceCount,
                         drawCommand.firstIndex,
                         drawCommand.vertexOffset,
                         drawCommand.firstInstance);
    }
}

void VulkanRenderPassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const std::vector<uint8_t> &data)
{
    VulkanGraphicsPipeline *vulkanPipeline = vulkanResourceManager->getGraphicsPipeline(pipeline);

    vkCmdPushConstants(commandBuffer,
                       vulkanPipeline->pipelineLayout,
                       shaderStageFlagBitsToVkShaderStageFlagBits(static_cast<ShaderStageFlagBits>(constantRange.shaderStages)),
                       constantRange.offset,
                       constantRange.size,
                       data.data());
}

void VulkanRenderPassCommandRecorder::end()
{
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace ToyRenderer
