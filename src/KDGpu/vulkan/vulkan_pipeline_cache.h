/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;
struct PipelineCache_t;

/**
 * @brief VulkanPipelineCache
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanPipelineCache {

    explicit VulkanPipelineCache(VkPipelineCache _pipelineCache,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle);

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;

    std::vector<uint8_t> getData() const;
    bool merge(const std::vector<Handle<PipelineCache_t>> &srcHandles) const;
};

} // namespace KDGpu
