/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_shader_module.h"

namespace KDGpu {

VulkanShaderModule::VulkanShaderModule(VkShaderModule _shaderModule,
                                       VulkanResourceManager *_vulkanResourceManager,
                                       const Handle<Device_t> _deviceHandle)
    : shaderModule(_shaderModule)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu
