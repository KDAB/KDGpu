#include "vulkan_render_pass_command_recorder.h"

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

void VulkanRenderPassCommandRecorder::setPipeline(const Handle<GraphicsPipeline_t> &pipeline)
{
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

void VulkanRenderPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroupH)
{
    VulkanBindGroup *bindGroup = vulkanResourceManager->getBindGroup(bindGroupH);
    // TODO: We need to have created the descriptor first, do we want that to be done internally or should that be exposed in
    // the API
    VkDescriptorSet set = bindGroup->descriptorSet; // Find DescriptorSet for group;

    // TODO: Add API to know all the dirty descriptors in

    // VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(buffer);

    // VkDescriptorBufferInfo uboBufferInfo;
    // uboBufferInfo.buffer = vulkanBuffer->buffer; // VkBuffer
    // uboBufferInfo.offset = 0;
    // // TODO: We need API to retrieve the size
    // // uboBufferInfo.range = vulkanBuffer->size();

    // VkDescriptorImageInfo imageInfo;
    // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // imageInfo.imageView = vulkanTextureView->imageView;
    // // TODO: Create Sampler
    // // imageInfo.sampler = vulkanTextureView.sampler;

    // VkWriteDescriptorSet descriptorWrite{};
    // descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descriptorWrite.dstSet = set;
    // descriptorWrite.dstBinding = index;
    // descriptorWrite.dstArrayElement = 0;
    // descriptorWrite.descriptorCount = 1;
    // descriptorWrite.pImageInfo = nullptr;
    // descriptorWrite.pTexelBufferView = nullptr;
    // descriptorWrite.pBufferInfo = &uboBufferInfo;
    // descriptorWrite.pTexelBufferView = nullptr;
    // descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    // auto device = vulkanResourceManager->getDevice(deviceHandle);
    // vkUpdateDescriptorSets(device->device, 1, &descriptorWrite, 0, nullptr);

    // Bind Descriptor Set
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE }; // TODO: Retrieve that
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
    // TODO: Expose the viewport and scissor setting commands
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

void VulkanRenderPassCommandRecorder::end()
{
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace ToyRenderer
