/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_graphics_pipeline.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct PipelineLayout_t;

/**
 * @brief VulkanGraphicsPipeline
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanGraphicsPipeline : public ApiGraphicsPipeline {
    explicit VulkanGraphicsPipeline(VkPipeline _pipeline,
                                    VkRenderPass _renderPass,
                                    VulkanResourceManager *_vulkanResourceManager,
                                    const Handle<Device_t> &_deviceHandle,
                                    const Handle<PipelineLayout_t> &_pipelineLayoutHandle);

    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    Handle<PipelineLayout_t> pipelineLayoutHandle;
};

} // namespace KDGpu
