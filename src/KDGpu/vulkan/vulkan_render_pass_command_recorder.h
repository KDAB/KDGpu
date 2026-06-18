/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/render_pass_command_recorder.h>
#include <KDGpu/vulkan/vulkan_render_pass_command_recorder_base.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanRenderPassCommandRecorder - Legacy VkRenderPass-based rendering backend
 * \ingroup vulkan
 *
 * Backend implementation for legacy VkRenderPass and VkFramebuffer based rendering.
 * This class inherits common functionality from VulkanRenderPassCommandRecorderBase
 * and adds subpass-specific methods for pre-Vulkan 1.3 rendering.
 *
 * \deprecated For new applications, use VulkanDynamicRenderPassCommandRecorder instead.
 */
struct KDGPU_EXPORT VulkanRenderPassCommandRecorder : public VulkanRenderPassCommandRecorderBase {
    explicit VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                             VkRect2D _renderArea,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle);

    void nextSubpass() const;
    void end() const override;
};

} // namespace KDGpu
