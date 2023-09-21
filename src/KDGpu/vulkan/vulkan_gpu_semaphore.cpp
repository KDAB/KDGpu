/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_gpu_semaphore.h"

namespace KDGpu {

VulkanGpuSemaphore::VulkanGpuSemaphore(VkSemaphore _semaphore,
                                       VulkanResourceManager *_vulkanResourceManager,
                                       const Handle<Device_t> &_deviceHandle,
                                       const HandleOrFD &_externalSemaphoreHandle)
    : ApiGpuSemaphore()
    , semaphore(_semaphore)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , m_externalSemaphoreHandle(_externalSemaphoreHandle)
{
}

HandleOrFD VulkanGpuSemaphore::externalSemaphoreHandle() const
{
    return m_externalSemaphoreHandle;
}

} // namespace KDGpu
