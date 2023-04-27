/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_queue.h>
#include <KDGpu/kdgpu_export.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct KDGPU_EXPORT VulkanQueue : public ApiQueue {
    explicit VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager);

    void waitUntilIdle() final;
    void submit(const SubmitOptions &options) final;
    std::vector<PresentResult> present(const PresentOptions &options) final;

    VkQueue queue{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
};

} // namespace KDGpu
