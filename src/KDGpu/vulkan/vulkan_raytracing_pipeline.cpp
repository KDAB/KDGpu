/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_raytracing_pipeline.h"

#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_adapter.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanRayTracingPipeline::VulkanRayTracingPipeline(VkPipeline _pipeline,
                                                   VulkanResourceManager *_vulkanResourceManager,
                                                   const Handle<Device_t> &_deviceHandle,
                                                   const Handle<PipelineLayout_t> &_pipelineLayoutHandle)
    : pipeline(_pipeline)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , pipelineLayoutHandle(_pipelineLayoutHandle)
{
}

std::vector<uint8_t> VulkanRayTracingPipeline::shaderGroupHandles(uint32_t firstGroup, uint32_t groupCount) const
{
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    VulkanAdapter *adapter = vulkanResourceManager->getAdapter(device->adapterHandle);
    const uint32_t handleSize = adapter->queryAdapterProperties().rayTracingProperties.shaderGroupHandleSize;

    std::vector<uint8_t> handlesData;
    handlesData.resize(groupCount * handleSize);

    if (device->vkGetRayTracingShaderGroupHandlesKHR)
        device->vkGetRayTracingShaderGroupHandlesKHR(device->device, pipeline, firstGroup, groupCount,
                                                     handlesData.size(), handlesData.data());

    return handlesData;
}

} // namespace KDGpu
