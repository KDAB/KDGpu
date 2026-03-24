/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_timeline_semaphore.h"

#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <spdlog/spdlog.h>

namespace KDGpu {

VulkanTimelineSemaphore::VulkanTimelineSemaphore(VkSemaphore _semaphore,
                                                 VulkanResourceManager *_vulkanResourceManager,
                                                 const Handle<Device_t> &_deviceHandle,
                                                 const HandleOrFD &_externalSemaphoreHandle)
    : semaphore(_semaphore)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , m_externalSemaphoreHandle(_externalSemaphoreHandle)

{
}

uint64_t VulkanTimelineSemaphore::value() const
{
    uint64_t counterValue = 0;
#if VK_KHR_timeline_semaphore
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkGetSemaphoreCounterValueKHR == nullptr) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Timeline semaphore counter value retrieval is not supported by the device");
        return 0;
    }
    vulkanDevice->vkGetSemaphoreCounterValueKHR(vulkanDevice->device, semaphore, &counterValue);
#endif
    return counterValue;
}

void VulkanTimelineSemaphore::signal(uint64_t value) const
{
#if VK_KHR_timeline_semaphore
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkSignalSemaphoreKHR == nullptr) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Timeline semaphore signal is not supported by the device");
        return;
    }
    VkSemaphoreSignalInfo signalInfo = {};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    signalInfo.pNext = nullptr;
    signalInfo.semaphore = semaphore;
    signalInfo.value = value;
    vulkanDevice->vkSignalSemaphoreKHR(vulkanDevice->device, &signalInfo);
#endif
}

TimelineSemaphoreWaitResult VulkanTimelineSemaphore::wait(uint64_t value) const
{
#if VK_KHR_timeline_semaphore
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkWaitSemaphoresKHR == nullptr) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Timeline semaphore wait is not supported by the device");
        return TimelineSemaphoreWaitResult::Error;
    }
    VkSemaphoreWaitInfo waitInfo = {};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = nullptr;
    waitInfo.flags = 0;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &semaphore;
    waitInfo.pValues = &value;
    constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();
    const VkResult result = vulkanDevice->vkWaitSemaphoresKHR(vulkanDevice->device, &waitInfo, timeout);
    switch (result) {
    case VK_SUCCESS:
        return TimelineSemaphoreWaitResult::Success;
    case VK_TIMEOUT:
        return TimelineSemaphoreWaitResult::Timeout;
    default:
        return TimelineSemaphoreWaitResult::Error;
    }
#else
    return TimelineSemaphoreWaitResult::Error;
#endif
}

HandleOrFD VulkanTimelineSemaphore::externalSemaphoreHandle() const
{
    return m_externalSemaphoreHandle;
}

} // namespace KDGpu
