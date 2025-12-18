/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group.h>
#include <KDGpu/compute_pass_command_recorder.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct ComputePipeline_t;
struct Device_t;

/**
 * @brief VulkanComputePassCommandRecorder
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanComputePassCommandRecorder {

    explicit VulkanComputePassCommandRecorder(VkCommandBuffer _commandBuffer,
                                              VulkanResourceManager *_vulkanResourceManager,
                                              const Handle<Device_t> &_deviceHandle);

    void setPipeline(const Handle<ComputePipeline_t> &pipeline);
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout,
                      std::span<const uint32_t> dynamicBufferOffsets) const;
    void dispatchCompute(const ComputeCommand &command) const;
    void dispatchCompute(std::span<const ComputeCommand> commands) const;
    void dispatchComputeIndirect(const ComputeCommandIndirect &command) const;
    void dispatchComputeIndirect(std::span<const ComputeCommandIndirect> commands) const;
    void pushConstant(const PushConstantRange &constantRange, const void *data) const;
    void pushBindGroup(uint32_t group,
                       std::span<const BindGroupEntry> bindGroupEntries,
                       const Handle<PipelineLayout_t> &pipelineLayout) const;
    void end() const;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<ComputePipeline_t> pipeline;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace KDGpu
