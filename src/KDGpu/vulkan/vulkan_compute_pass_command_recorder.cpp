/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_compute_pass_command_recorder.h"
#include <KDGpu/vulkan/vulkan_compute_pipeline.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_enums.h>

namespace KDGpu {

VulkanComputePassCommandRecorder::VulkanComputePassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                   VulkanResourceManager *_vulkanResourceManager,
                                                                   const Handle<Device_t> &_deviceHandle)
    : commandBuffer(_commandBuffer)
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

void VulkanComputePassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &_bindGroup,
                                                    const Handle<PipelineLayout_t> &pipelineLayout, const std::vector<uint32_t> &dynamicBufferOffsets)
{
    VulkanBindGroup *bindGroup = vulkanResourceManager->getBindGroup(_bindGroup);
    VkDescriptorSet set = bindGroup->descriptorSet;

    // Use the pipeline layout provided, otherwise fallback to the one from the currently
    // bound pipeline (if any).
    VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
    if (pipelineLayout.isValid()) {
        VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(pipelineLayout);
        if (vulkanPipelineLayout)
            vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
    } else if (pipeline.isValid()) {
        VulkanComputePipeline *vulkanPipeline = vulkanResourceManager->getComputePipeline(pipeline);
        if (vulkanPipeline) {
            VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);
            if (vulkanPipelineLayout)
                vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
        }
    }

    assert(vkPipelineLayout != VK_NULL_HANDLE); // The PipelineLayout should outlive the pipelines

    // Bind Descriptor Set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            vkPipelineLayout,
                            group,
                            1, &set,
                            dynamicBufferOffsets.size(), dynamicBufferOffsets.data());
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

void VulkanComputePassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data)
{
    VulkanComputePipeline *vulkanPipeline = vulkanResourceManager->getComputePipeline(pipeline);
    VulkanPipelineLayout *pLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);

    assert(pLayout != nullptr); // The PipelineLayout should outlive the pipelines
    vkCmdPushConstants(commandBuffer,
                       pLayout->pipelineLayout,
                       constantRange.shaderStages.toInt(),
                       constantRange.offset,
                       constantRange.size,
                       data);
}

void VulkanComputePassCommandRecorder::end()
{
    // No op
}

} // namespace KDGpu
