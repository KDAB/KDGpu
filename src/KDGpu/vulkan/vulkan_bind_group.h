/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_bind_group.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;

/**
 * @brief VulkanBindGroup
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanBindGroup : public ApiBindGroup {
    explicit VulkanBindGroup(VkDescriptorSet _descriptorSet,
                             VkDescriptorPool _descriptorPool,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle);

    void update(const BindGroupEntry &entry) final;

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
