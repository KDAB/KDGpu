/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_bind_group_pool.h"
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanBindGroupPool::VulkanBindGroupPool(VkDescriptorPool _descriptorPool,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         const Handle<Device_t> &_deviceHandle)
    : descriptorPool(_descriptorPool), vulkanResourceManager(_vulkanResourceManager), deviceHandle(_deviceHandle)
{
}

void VulkanBindGroupPool::reset()
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vkResetDescriptorPool(vulkanDevice->device, descriptorPool, VkDescriptorPoolResetFlags{});

    // TODO: Release all associated BindGroups
}

} // namespace KDGpu
