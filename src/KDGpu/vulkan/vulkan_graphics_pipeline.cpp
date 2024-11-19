/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_graphics_pipeline.h"

namespace KDGpu {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPipeline _pipeline,
                                               VulkanResourceManager *_vulkanResourceManager,
                                               const Handle<RenderPass_t> &_renderPassHandle,
                                               const std::vector<VkDynamicState> &_enabledDynamicState,
                                               const Handle<Device_t> &_deviceHandle,
                                               const Handle<PipelineLayout_t> &_pipelineLayoutHandle)
    : pipeline(_pipeline)
    , vulkanResourceManager(_vulkanResourceManager)
    , renderPassHandle(_renderPassHandle)
    , enabledDynamicState(_enabledDynamicState)
    , deviceHandle(_deviceHandle)
    , pipelineLayoutHandle(_pipelineLayoutHandle)
{
}

} // namespace KDGpu
