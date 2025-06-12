/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_bind_group_pool.h"
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <algorithm>
#include <vector>

namespace KDGpu {

VulkanBindGroupPool::VulkanBindGroupPool(VkDescriptorPool _descriptorPool,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         const Handle<Device_t> &_deviceHandle,
                                         uint16_t _maxBindGroupCount)
    : descriptorPool(_descriptorPool), vulkanResourceManager(_vulkanResourceManager), deviceHandle(_deviceHandle), maxBindGroupCount(_maxBindGroupCount)
{
}

void VulkanBindGroupPool::reset()
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vkResetDescriptorPool(vulkanDevice->device, descriptorPool, VkDescriptorPoolResetFlags{});

    // Reset vulkan descriptor set handle on the referenced bind groups since they have been reset by the pool
    for (const auto &bindGroupHandle : m_bindGroups) {
        VulkanBindGroup *bindGroup = vulkanResourceManager->getBindGroup(bindGroupHandle);
        assert(bindGroup); // Should not ever happen but in case it ever does
        bindGroup->descriptorSet = VK_NULL_HANDLE;
    }

    // Clear the tracked bind groups as they are now invalidated
    m_bindGroups.clear();
}

void VulkanBindGroupPool::addBindGroup(const Handle<BindGroup_t> &bindGroupHandle)
{
    m_bindGroups.push_back(bindGroupHandle);
}

void VulkanBindGroupPool::removeBindGroup(const Handle<BindGroup_t> &bindGroupHandle)
{
    std::erase(m_bindGroups, bindGroupHandle);
}

const std::vector<Handle<BindGroup_t>> &VulkanBindGroupPool::bindGroups() const
{
    return m_bindGroups;
}

uint16_t VulkanBindGroupPool::bindGroupCount() const
{
    return static_cast<uint16_t>(m_bindGroups.size());
}

} // namespace KDGpu
