/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/vulkan/vulkan_render_pass_command_recorder_base.h>

#include <vulkan/vulkan.h>

#include <span>
#include <optional>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanDynamicRenderPassCommandRecorder - Modern dynamic rendering backend
 * \ingroup vulkan
 *
 * Backend implementation for modern Vulkan 1.3 core VK_KHR_dynamic_rendering.
 * This class inherits common functionality from VulkanRenderPassCommandRecorderBase
 * and provides a clean separation from legacy VkRenderPass-based rendering.
 *
 * Recommended for all new applications using Vulkan 1.3 or later.
 */
struct KDGPU_EXPORT VulkanDynamicRenderPassCommandRecorder : public VulkanRenderPassCommandRecorderBase {
    explicit VulkanDynamicRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                    VkRect2D _renderArea,
                                                    VulkanResourceManager *_vulkanResourceManager,
                                                    const Handle<Device_t> &_deviceHandle);

    void setInputAttachmentMapping(std::span<const std::optional<uint32_t>> colorAttachmentIndices,
                                   std::optional<uint32_t> depthAttachmentIndex,
                                   std::optional<uint32_t> stencilAttachmentIndex) const;
    void setOutputAttachmentMapping(std::span<const std::optional<uint32_t>> remappedOutputs) const;
    void end() const override;
};

} // namespace KDGpu
