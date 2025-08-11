/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct PipelineLayout_t;
struct RenderPass_t;

/**
 * @brief VulkanGraphicsPipeline
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanGraphicsPipeline {
    explicit VulkanGraphicsPipeline(VkPipeline _pipeline,
                                    VulkanResourceManager *_vulkanResourceManager,
                                    const Handle<RenderPass_t> &_renderPassHandle,
                                    const std::vector<VkDynamicState> &_enabledDynamicState,
                                    const Handle<Device_t> &_deviceHandle,
                                    const Handle<PipelineLayout_t> &_pipelineLayoutHandle,
                                    bool _dynamicRendering);

    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkRenderPass renderPass{ VK_NULL_HANDLE };
    std::vector<VkDynamicState> enabledDynamicState;

    VulkanResourceManager *vulkanResourceManager;
    Handle<RenderPass_t> renderPassHandle;
    Handle<Device_t> deviceHandle;
    Handle<PipelineLayout_t> pipelineLayoutHandle;
    bool dynamicRendering{ false };
};

} // namespace KDGpu
