/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_timestamp_query_recorder.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

struct KDGPU_EXPORT VulkanTimestampQueryRecorder : public ApiTimestampQueryRecorder {

    explicit VulkanTimestampQueryRecorder(VkCommandBuffer _commandBuffer,
                                          VulkanResourceManager *_vulkanResourceManager,
                                          const Handle<Device_t> &_deviceHandle,
                                          uint32_t _startQuery,
                                          uint32_t _maxQueryCount);

    TimestampIndex writeTimestamp(PipelineStageFlags flags) final;
    std::vector<uint64_t> queryResults() final;
    void reset() final;
    float timestampPeriod() const final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    uint32_t queryCount{ 0 };
    uint32_t startQuery;
    uint32_t maxQueryCount;
    float m_timestampPeriod{ 1.0f };
};

} // namespace KDGpu
