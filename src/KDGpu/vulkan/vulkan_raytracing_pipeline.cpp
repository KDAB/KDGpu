/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_raytracing_pipeline.h"

namespace KDGpu {

VulkanRayTracingPipeline::VulkanRayTracingPipeline(VkPipeline _pipeline,
                                                   VulkanResourceManager *_vulkanResourceManager,
                                                   const Handle<Device_t> &_deviceHandle,
                                                   const Handle<PipelineLayout_t> &_pipelineLayoutHandle)
    : ApiRayTracingPipeline()
    , pipeline(_pipeline)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , pipelineLayoutHandle(_pipelineLayoutHandle)
{
}

} // namespace KDGpu
