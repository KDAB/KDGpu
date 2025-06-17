/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group_options.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;
struct BindGroupPool_t;

/**
 * @brief VulkanBindGroup
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanBindGroup {
    explicit VulkanBindGroup(VkDescriptorSet _descriptorSet,
                             const Handle<BindGroupPool_t> &_bindGroupPoolHandle,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle,
                             bool _implicitFree);

    void update(const BindGroupEntry &entry);
    bool hasValidHandle() const { return descriptorSet != VK_NULL_HANDLE; };

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    Handle<BindGroupPool_t> bindGroupPoolHandle;
    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    bool implicitFree{ false };
};

} // namespace KDGpu
