/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_compute_pipeline.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct PipelineLayout_t;

struct KDGPU_EXPORT VulkanComputePipeline : public ApiComputePipeline {
    explicit VulkanComputePipeline(VkPipeline _pipeline,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle,
                                   const Handle<PipelineLayout_t> &_pipelineLayoutHandle);

    VkPipeline pipeline{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    Handle<PipelineLayout_t> pipelineLayoutHandle;
};

} // namespace KDGpu
