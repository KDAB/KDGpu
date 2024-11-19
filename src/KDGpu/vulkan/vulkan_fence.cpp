/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_fence.h"
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanFence::VulkanFence(VkFence _fence,
                         VulkanResourceManager *_vulkanResourceManager,
                         const Handle<Device_t> &_deviceHandle,
                         const HandleOrFD &_externalFenceHandle)
    : fence(_fence)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , m_externalFenceHandle(_externalFenceHandle)
{
}

void VulkanFence::wait()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vkWaitForFences(vulkanDevice->device, 1, &fence, true, std::numeric_limits<uint64_t>::max());
}

void VulkanFence::reset()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vkResetFences(vulkanDevice->device, 1, &fence);
}

FenceStatus VulkanFence::status()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    VkResult vkResult = vkGetFenceStatus(vulkanDevice->device, fence);
    switch (vkResult) {
    case VK_SUCCESS:
        return FenceStatus::Signalled;
    case VK_NOT_READY:
        return FenceStatus::Unsignalled;
    default:
        return FenceStatus::Error;
    }
}

HandleOrFD VulkanFence::externalFenceHandle() const
{
    return m_externalFenceHandle;
}

} // namespace KDGpu
