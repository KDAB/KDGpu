/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_render_pass.h"

namespace KDGpu {

VulkanRenderPass::VulkanRenderPass(VkRenderPass _renderPass,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   Handle<Device_t> _deviceHandle)
    : ApiRenderPass()
    , renderPass(_renderPass)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu
