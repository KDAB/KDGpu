/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/queue.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

/**
 * @brief VulkanQueue
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanQueue {
    explicit VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager);

    void waitUntilIdle();
    void submit(const SubmitOptions &options);
    PresentResult present(const PresentOptions &options);
    std::vector<PresentResult> lastPerSwapchainPresentResults() const;

    VkQueue queue{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };

    // Presentation
    std::vector<VkResult> m_presentResults;
};

} // namespace KDGpu
