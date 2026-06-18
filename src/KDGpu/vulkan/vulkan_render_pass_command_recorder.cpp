/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_render_pass_command_recorder.h"

#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanRenderPassCommandRecorder::VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                 VkRect2D _renderArea,
                                                                 VulkanResourceManager *_vulkanResourceManager,
                                                                 const Handle<Device_t> &_deviceHandle)
    : VulkanRenderPassCommandRecorderBase(_commandBuffer, _renderArea, _vulkanResourceManager, _deviceHandle)
{
}

void VulkanRenderPassCommandRecorder::nextSubpass() const
{
    // For now we assume renderpass/subpass are always recorded inline (primary command buffer)
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPassCommandRecorder::end() const
{
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace KDGpu
