/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_pipeline_layout.h"

namespace KDGpu {

VulkanPipelineLayout::VulkanPipelineLayout(VkPipelineLayout _pipelineLayout,
                                           std::vector<VkDescriptorSetLayout> &&_descriptorSetLayouts,
                                           VulkanResourceManager *_vulkanResourceManager,
                                           const Handle<Device_t> &_deviceHandle)
    : pipelineLayout(_pipelineLayout)
    , descriptorSetLayouts(_descriptorSetLayouts)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu
