/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

/**
 * @brief VulkanTimelineSemaphore
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanTimelineSemaphore {
    explicit VulkanTimelineSemaphore(VkSemaphore _semaphore,
                                     VulkanResourceManager *_vulkanResourceManager,
                                     const Handle<Device_t> &_deviceHandle,
                                     const HandleOrFD &_externalSemaphoreHandle);

    VkSemaphore semaphore{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    HandleOrFD m_externalSemaphoreHandle;

    uint64_t value() const;
    void signal(uint64_t value);
    TimelineSemaphoreWaitResult wait(uint64_t value) const;
    HandleOrFD externalSemaphoreHandle() const;
};

} // namespace KDGpu
