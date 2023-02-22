#include "vulkan_render_pass_command_recorder.h"

#include <toy_renderer/vulkan/vulkan_graphics_pipeline.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

#include <array>

namespace ToyRenderer {

VulkanRenderPassCommandRecorder::VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                 VulkanResourceManager *_vulkanResourceManager,
                                                                 const Handle<Device_t> &_deviceHandle)
    : ApiRenderPassCommandRecorder()
    , commandBuffer(_commandBuffer)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
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

void VulkanRenderPassCommandRecorder::end()
{
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace ToyRenderer
