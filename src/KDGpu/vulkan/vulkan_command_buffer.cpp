/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_command_buffer.h"

#include <KDGpu/utils/logging.h>
#include <KDGpu/vulkan/vulkan_formatters.h>

namespace KDGpu {

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer _commandBuffer,
                                         VkCommandPool _commandPool,
                                         VkCommandBufferLevel _commandLevel,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         const Handle<Device_t> &_deviceHandle)
    : commandBuffer(_commandBuffer)
    , commandPool(_commandPool)
    , commandLevel(_commandLevel)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanCommandBuffer::begin()
{
    // Begin recording
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    if (commandLevel == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

        // TODO: We will need a way to specify the RenderPass/FrameBuffer if we want to record
        // commands from within a RenderPass.
        beginInfo.pInheritanceInfo = &inheritanceInfo;
    }

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Unable to begin command buffer recording: {}", result);
    }
}

void VulkanCommandBuffer::finish()
{
    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Unable to end command buffer recording: {}", result);
    }
}

} // namespace KDGpu
