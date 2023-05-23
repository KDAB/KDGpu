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

/**
 * @brief VulkanQueue
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanQueue : public ApiQueue {
    explicit VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager);

    void waitUntilIdle() final;
    void submit(const SubmitOptions &options) final;
    PresentResult present(const PresentOptions &options) final;
    std::vector<PresentResult> lastPerSwapchainPresentResults() const final;

    VkQueue queue{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };

    // Submission
    std::vector<VkSemaphore> m_vkWaitSemaphores;
    std::vector<VkPipelineStageFlags> m_vkWaitStageFlags;
    std::vector<VkSemaphore> m_vkSignalSemaphores;
    std::vector<VkCommandBuffer> m_vkCommandBuffers;

    // Presentation
    std::vector<VkSemaphore> m_presentVkWaitSemaphores;
    std::vector<VkSwapchainKHR> m_swapchains;
    std::vector<uint32_t> m_imageIndices;
    std::vector<VkResult> m_presentResults;
};

} // namespace KDGpu
