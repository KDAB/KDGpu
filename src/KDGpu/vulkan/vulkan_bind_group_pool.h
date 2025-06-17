/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/bind_group_pool_options.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;
struct BindGroup_t;

/**
 * @brief VulkanBindGroupPool
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanBindGroupPool {
    explicit VulkanBindGroupPool(VkDescriptorPool _descriptorPool,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle,
                                 uint16_t _maxBindGroupCount,
                                 BindGroupPoolFlags _flags);

    void reset();

    // Methods to track allocated bind groups
    void addBindGroup(const Handle<BindGroup_t> &bindGroupHandle);
    void removeBindGroup(const Handle<BindGroup_t> &bindGroupHandle);
    const std::vector<Handle<BindGroup_t>> &bindGroups() const;
    uint16_t bindGroupCount() const;

    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    uint16_t maxBindGroupCount;
    BindGroupPoolFlags flags{ BindGroupPoolFlagBits::None };

private:
    std::vector<Handle<BindGroup_t>> m_bindGroups;
};

} // namespace KDGpu
