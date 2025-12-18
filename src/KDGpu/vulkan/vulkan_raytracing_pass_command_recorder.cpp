/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_raytracing_pass_command_recorder.h"
#include <KDGpu/vulkan/vulkan_raytracing_pipeline.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/bind_group_options.h>

#include <vector>

namespace KDGpu {

VulkanRayTracingPassCommandRecorder::VulkanRayTracingPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                         VulkanResourceManager *_vulkanResourceManager,
                                                                         const Handle<Device_t> &_deviceHandle)
    : commandBuffer(_commandBuffer)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanRayTracingPassCommandRecorder::setPipeline(const Handle<RayTracingPipeline_t> &_pipeline)
{
    pipeline = _pipeline;
    VulkanRayTracingPipeline *vulkanPipeline = vulkanResourceManager->getRayTracingPipeline(pipeline);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, vulkanPipeline->pipeline);
}

void VulkanRayTracingPassCommandRecorder::setBindGroup(uint32_t group, const Handle<BindGroup_t> &_bindGroup,
                                                       const Handle<PipelineLayout_t> &pipelineLayout,
                                                       std::span<const uint32_t> dynamicBufferOffsets) const
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
        VulkanRayTracingPipeline *vulkanPipeline = vulkanResourceManager->getRayTracingPipeline(pipeline);
        if (vulkanPipeline) {
            VulkanPipelineLayout *vulkanPipelineLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);
            if (vulkanPipelineLayout)
                vkPipelineLayout = vulkanPipelineLayout->pipelineLayout;
        }
    }

    assert(vkPipelineLayout != VK_NULL_HANDLE); // The PipelineLayout should outlive the pipelines

    // Bind Descriptor Set
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                            vkPipelineLayout,
                            group,
                            1, &set,
                            dynamicBufferOffsets.size(), dynamicBufferOffsets.data());
}

namespace {

VkStridedDeviceAddressRegionKHR buildVkStridedDeviceAddressRegion(
        VulkanResourceManager *vulkanResourceManager,
        const StridedDeviceRegion &region)
{
    VkStridedDeviceAddressRegionKHR vkRegion{};
    if (region.buffer.isValid()) {
        VulkanBuffer *vulkanBuffer = vulkanResourceManager->getBuffer(region.buffer);
        VkBufferDeviceAddressInfo addressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
        addressInfo.buffer = vulkanBuffer->buffer;

        vkRegion.deviceAddress = vulkanBuffer->bufferDeviceAddress() + region.offset;
        vkRegion.size = region.size;
        vkRegion.stride = region.stride;
    };
    return vkRegion;
}

} // namespace

void VulkanRayTracingPassCommandRecorder::traceRays(const RayTracingCommand &rayTracingCommand) const
{
#if defined(VK_KHR_ray_tracing_pipeline)
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdTraceRaysKHR) {

        VkStridedDeviceAddressRegionKHR raygenShaderBindingTableRegion = buildVkStridedDeviceAddressRegion(vulkanResourceManager, rayTracingCommand.raygenShaderBindingTable);
        VkStridedDeviceAddressRegionKHR hitShaderBindingTableRegion = buildVkStridedDeviceAddressRegion(vulkanResourceManager, rayTracingCommand.hitShaderBindingTable);
        VkStridedDeviceAddressRegionKHR missShaderBindingTableRegion = buildVkStridedDeviceAddressRegion(vulkanResourceManager, rayTracingCommand.missShaderBindingTable);
        VkStridedDeviceAddressRegionKHR callableShaderBindingTableRegion = buildVkStridedDeviceAddressRegion(vulkanResourceManager, rayTracingCommand.callableShaderBindingTable);

        device->vkCmdTraceRaysKHR(commandBuffer,
                                  &raygenShaderBindingTableRegion,
                                  &missShaderBindingTableRegion,
                                  &hitShaderBindingTableRegion,
                                  &callableShaderBindingTableRegion,
                                  rayTracingCommand.extent.width,
                                  rayTracingCommand.extent.height,
                                  rayTracingCommand.extent.depth);
    }
#else
    assert(false);
#endif
}

void VulkanRayTracingPassCommandRecorder::pushConstant(const PushConstantRange &constantRange, const void *data) const
{
    VulkanRayTracingPipeline *vulkanPipeline = vulkanResourceManager->getRayTracingPipeline(pipeline);
    VulkanPipelineLayout *pLayout = vulkanResourceManager->getPipelineLayout(vulkanPipeline->pipelineLayoutHandle);

    assert(pLayout != nullptr); // The PipelineLayout should outlive the pipelines
    vkCmdPushConstants(commandBuffer,
                       pLayout->pipelineLayout,
                       constantRange.shaderStages.toInt(),
                       constantRange.offset,
                       constantRange.size,
                       data);
}

void VulkanRayTracingPassCommandRecorder::pushBindGroup(uint32_t group,
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
            VulkanRayTracingPipeline *vulkanPipeline = vulkanResourceManager->getRayTracingPipeline(pipeline);
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
                                          VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                          vkPipelineLayout,
                                          group,
                                          writeDescriptorSets.size(),
                                          writeDescriptorSets.data());
    }
#else
    assert(false);
#endif
}

void VulkanRayTracingPassCommandRecorder::end() const
{
    // No op
}

} // namespace KDGpu
