#include "vulkan_compute_pass_command_recorder.h"
#include <toy_renderer/vulkan/vulkan_compute_pipeline.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>
#include <toy_renderer/vulkan/vulkan_enums.h>

namespace ToyRenderer {

VulkanComputePassCommandRecorder::VulkanComputePassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                   VulkanResourceManager *_vulkanResourceManager,
                                                                   const Handle<Device_t> &_deviceHandle)
    : ApiComputePassCommandRecorder()
    , commandBuffer(_commandBuffer)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanComputePassCommandRecorder::setPipeline(const Handle<ComputePipeline_t> &_pipeline)
{
    pipeline = _pipeline;
    VulkanComputePipeline *vulkanPipeline = vulkanResourceManager->getComputePipeline(pipeline);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipeline->pipeline);
}

void VulkanComputePassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &_bindGroup)
{
    VulkanBindGroup *bindGroup = vulkanResourceManager->getBindGroup(_bindGroup);
    VkDescriptorSet set = bindGroup->descriptorSet;

    // Bind Descriptor Set
    VulkanComputePipeline *p = vulkanResourceManager->getComputePipeline(pipeline);
    VkPipelineLayout pipelineLayout = p->pipelineLayout;
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout,
                            group,
                            1, &set,
                            0, nullptr);
}

void VulkanComputePassCommandRecorder::dispatchCompute(const ComputeCommand &command)
{
    vkCmdDispatch(commandBuffer, command.workGroupX, command.workGroupY, command.workGroupZ);
}

void VulkanComputePassCommandRecorder::dispatchCompute(const std::vector<ComputeCommand> &commands)
{
    for (const auto &c : commands)
        dispatchCompute(c);
}

void VulkanComputePassCommandRecorder::dispatchComputeIndirect(const ComputeCommandIndirect &command)
{
    VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(command.buffer);
    vkCmdDispatchIndirect(commandBuffer, vulkanBuffer->buffer, command.offset);
}

void VulkanComputePassCommandRecorder::dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands)
{
    for (const auto &c : commands)
        dispatchComputeIndirect(c);
}

void VulkanComputePassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const std::vector<uint8_t> &data)
{
    VulkanComputePipeline *vulkanPipeline = vulkanResourceManager->getComputePipeline(pipeline);

    vkCmdPushConstants(commandBuffer,
                       vulkanPipeline->pipelineLayout,
                       shaderStageFlagBitsToVkShaderStageFlagBits(static_cast<ShaderStageFlagBits>(constantRange.shaderStages)),
                       constantRange.offset,
                       constantRange.size,
                       data.data());
}

void VulkanComputePassCommandRecorder::end()
{
    // No op
}

} // namespace ToyRenderer
