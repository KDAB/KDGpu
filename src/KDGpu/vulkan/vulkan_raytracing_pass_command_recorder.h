/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/pipeline_layout.h>
#include <KDGpu/raytracing_pass_command_recorder.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct RayTracingPipeline_t;
struct Device_t;

/**
 * @brief VulkanRayTracingPassCommandRecorder
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanRayTracingPassCommandRecorder {

    explicit VulkanRayTracingPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                 VulkanResourceManager *_vulkanResourceManager,
                                                 const Handle<Device_t> &_deviceHandle);

    void setPipeline(const Handle<RayTracingPipeline_t> &pipeline);
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout, std::span<const uint32_t> dynamicBufferOffsets);
    void traceRays(const RayTracingCommand &rayTracingCommand);
    void pushConstant(const PushConstantRange &constantRange, const void *data);
    void pushBindGroup(uint32_t group,
                       std::span<const BindGroupEntry> bindGroupEntries,
                       const Handle<PipelineLayout_t> &pipelineLayout);
    void end();

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<RayTracingPipeline_t> pipeline;
};

} // namespace KDGpu
