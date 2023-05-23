/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_queue.h"

#include <KDGpu/queue.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanQueue::VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager)
    : ApiQueue()
    , queue(_queue)
    , vulkanResourceManager(_vulkanResourceManager)
{
}

void VulkanQueue::waitUntilIdle()
{
    vkQueueWaitIdle(queue);
}

void VulkanQueue::submit(const SubmitOptions &options)
{
    constexpr size_t MaxWaitSemaphore = 10;
    // TODO: Do we need to expose the wait stage flags to the public API or is waiting
    // for the semaphores at the top of the pipeline good enough?
    const uint32_t waitSemaphoreCount = static_cast<uint32_t>(options.waitSemaphores.size());
    m_vkWaitSemaphores.clear();
    m_vkWaitStageFlags.clear();
    m_vkWaitSemaphores.reserve(waitSemaphoreCount);
    m_vkWaitStageFlags.reserve(waitSemaphoreCount);
    for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.waitSemaphores[i]);
        if (vulkanSemaphore) {
            m_vkWaitSemaphores.emplace_back(vulkanSemaphore->semaphore);
            m_vkWaitStageFlags.emplace_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
    }

    const uint32_t signalSemaphoreCount = static_cast<uint32_t>(options.signalSemaphores.size());
    m_vkSignalSemaphores.clear();
    m_vkSignalSemaphores.reserve(signalSemaphoreCount);
    for (uint32_t i = 0; i < signalSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.signalSemaphores[i]);
        if (vulkanSemaphore)
            m_vkSignalSemaphores.emplace_back(vulkanSemaphore->semaphore);
    }

    const uint32_t commandBufferCount = static_cast<uint32_t>(options.commandBuffers.size());
    m_vkCommandBuffers.clear();
    m_vkCommandBuffers.reserve(commandBufferCount);
    for (uint32_t i = 0; i < commandBufferCount; ++i) {
        auto vulkanCommandBuffer = vulkanResourceManager->getCommandBuffer(options.commandBuffers[i]);
        if (vulkanCommandBuffer)
            m_vkCommandBuffers.emplace_back(vulkanCommandBuffer->commandBuffer);
    }

    VkFence vkFenceToSignal{ VK_NULL_HANDLE };
    VulkanFence *vulkanFence = vulkanResourceManager->getFence(options.signalFence);
    if (vulkanFence)
        vkFenceToSignal = vulkanFence->fence;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = m_vkWaitSemaphores.size();
    submitInfo.pWaitSemaphores = m_vkWaitSemaphores.data();
    submitInfo.pWaitDstStageMask = m_vkWaitStageFlags.data();

    submitInfo.signalSemaphoreCount = m_vkSignalSemaphores.size();
    submitInfo.pSignalSemaphores = m_vkSignalSemaphores.data();

    submitInfo.commandBufferCount = m_vkCommandBuffers.size();
    submitInfo.pCommandBuffers = m_vkCommandBuffers.data();

    // TODO: Support fences
    // Make sure the fence is ready for use and submit
    // VkFence inFlightFences[] = { frameFence };
    // vkResetFences(renderer()->vulkanDevice()->device(), 1, inFlightFences);

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, vkFenceToSignal);
}

namespace {

auto mapVkResultToPresentResult = [](const VkResult r) {
    switch (r) {
    case VK_ERROR_OUT_OF_HOST_MEMORY:
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return PresentResult::OutOfMemory;
    case VK_ERROR_DEVICE_LOST:
        return PresentResult::DeviceLost;
    case VK_ERROR_OUT_OF_DATE_KHR:
        return PresentResult::OutOfDate;
    case VK_ERROR_SURFACE_LOST_KHR:
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return PresentResult::SurfaceLost;
    case VK_SUBOPTIMAL_KHR:
    case VK_SUCCESS:
    default:
        return PresentResult::Success;
    }
};

} // namespace

PresentResult VulkanQueue::present(const PresentOptions &options)
{
    const uint32_t waitSemaphoreCount = static_cast<uint32_t>(options.waitSemaphores.size());
    m_presentVkWaitSemaphores.clear();
    m_presentVkWaitSemaphores.reserve(waitSemaphoreCount);
    for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.waitSemaphores.at(i));
        if (vulkanSemaphore)
            m_presentVkWaitSemaphores.push_back(vulkanSemaphore->semaphore);
    }

    const uint32_t swapchainCount = static_cast<uint32_t>(options.swapchainInfos.size());
    m_swapchains.clear();
    m_imageIndices.clear();
    m_swapchains.reserve(swapchainCount);
    m_imageIndices.reserve(swapchainCount);
    for (uint32_t i = 0; i < swapchainCount; ++i) {
        auto vulkanSwapchain = vulkanResourceManager->getSwapchain(options.swapchainInfos.at(i).swapchain);
        if (vulkanSwapchain) {
            m_swapchains.push_back(vulkanSwapchain->swapchain);
            m_imageIndices.push_back(options.swapchainInfos.at(i).imageIndex);
        }
    }

    m_presentResults.resize(m_swapchains.size());
    m_presentResults.clear();

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(m_presentVkWaitSemaphores.size());
    presentInfo.pWaitSemaphores = m_presentVkWaitSemaphores.data();
    presentInfo.swapchainCount = static_cast<uint32_t>(m_swapchains.size());
    presentInfo.pSwapchains = m_swapchains.data();
    presentInfo.pImageIndices = m_imageIndices.data();
    presentInfo.pResults = m_presentResults.data();

    const VkResult result = vkQueuePresentKHR(queue, &presentInfo);
    return mapVkResultToPresentResult(result);
}

std::vector<PresentResult> VulkanQueue::lastPerSwapchainPresentResults() const
{
    // Else take time to convert the values
    std::vector<PresentResult> out;
    out.reserve(m_presentResults.size());

    for (VkResult r : m_presentResults)
        out.emplace_back(mapVkResultToPresentResult(r));

    return out;
}

} // namespace KDGpu
