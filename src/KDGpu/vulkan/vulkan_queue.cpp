/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_queue.h"

#include <KDGpu/queue.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_formatters.h>
#include <KDGpu/vulkan/vulkan_enums.h>

namespace KDGpu {

VulkanQueue::VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager)
    : queue(_queue)
    , vulkanResourceManager(_vulkanResourceManager)
{
}

void VulkanQueue::waitUntilIdle()
{
    vkQueueWaitIdle(queue);
}

void VulkanQueue::submit(const SubmitOptions &options)
{
    const uint32_t waitBinaryCount = static_cast<uint32_t>(options.waitSemaphores.size());
    const uint32_t waitTimelineCount = static_cast<uint32_t>(options.waitTimelineSemaphores.size());
    std::vector<VkSemaphore> vkWaitSemaphores;
    vkWaitSemaphores.reserve(waitBinaryCount + waitTimelineCount);

    std::vector<VkPipelineStageFlags> vkWaitStageFlags;
    vkWaitStageFlags.reserve(waitBinaryCount + waitTimelineCount);

    const uint32_t signalTimelineCount = static_cast<uint32_t>(options.signalTimelineSemaphores.size());
    const uint32_t signalBinaryCount = static_cast<uint32_t>(options.signalSemaphores.size());
    std::vector<VkSemaphore> vkSignalSemaphores;
    vkSignalSemaphores.reserve(signalBinaryCount + signalTimelineCount);

    // Even if signal/wait values are meaningless for binary semaphores, we will still need to provide a value
    // (which the implementation will ignore) since VkTimelineSemaphoreSubmitInfo requires that the wait and signal semaphore value vectors
    //  are the same length as the corresponding semaphore vectors in VkSubmitInfo.
    std::vector<uint64_t> vkWaitSemaphoreValues;
    vkWaitSemaphoreValues.reserve(waitBinaryCount + waitTimelineCount);
    std::vector<uint64_t> vkSignalSemaphoreValues;
    vkSignalSemaphoreValues.reserve(signalBinaryCount + signalTimelineCount);

    // Fill Wait and Signal Binary Semaphores
    for (const BinarySemaphoreSubmitWaitInfo &waitInfo : options.waitSemaphores) {
        VulkanGpuSemaphore *vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(waitInfo.semaphore);
        if (vulkanSemaphore) {
            vkWaitSemaphores.emplace_back(vulkanSemaphore->semaphore);
            vkWaitStageFlags.emplace_back(pipelineStageFlagsToVkPipelineStageFlagBits(waitInfo.waitStages));
            vkWaitSemaphoreValues.emplace_back(0);
        }
    }
    for (const RequiredHandle<GpuSemaphore_t> &signalSemaphoreHandle : options.signalSemaphores) {
        VulkanGpuSemaphore *vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(signalSemaphoreHandle);
        if (vulkanSemaphore) {
            vkSignalSemaphores.emplace_back(vulkanSemaphore->semaphore);
            vkSignalSemaphoreValues.emplace_back(0);
        }
    }

#if VK_KHR_timeline_semaphore
    // Fill Wait and Signal Timeline Semaphores
    for (const TimelineSemaphoreSubmitWaitInfo &waitInfo : options.waitTimelineSemaphores) {
        VulkanTimelineSemaphore *vulkanSemaphore = vulkanResourceManager->getTimelineSemaphore(waitInfo.semaphore);
        if (vulkanSemaphore) {
            vkWaitSemaphores.emplace_back(vulkanSemaphore->semaphore);
            vkWaitStageFlags.emplace_back(pipelineStageFlagsToVkPipelineStageFlagBits(waitInfo.waitStages));
            vkWaitSemaphoreValues.emplace_back(waitInfo.value);
        }
    }
    for (const TimelineSemaphoreSubmitSignalInfo &signalInfo : options.signalTimelineSemaphores) {
        VulkanTimelineSemaphore *vulkanSemaphore = vulkanResourceManager->getTimelineSemaphore(signalInfo.semaphore);
        if (vulkanSemaphore) {
            vkSignalSemaphores.emplace_back(vulkanSemaphore->semaphore);
            vkSignalSemaphoreValues.emplace_back(signalInfo.value);
        }
    }
#endif

    std::vector<VkCommandBuffer> vkCommandBuffers;
    const uint32_t commandBufferCount = static_cast<uint32_t>(options.commandBuffers.size());
    vkCommandBuffers.reserve(commandBufferCount);
    for (const auto &commandBufferHandle : options.commandBuffers) {
        VulkanCommandBuffer *vulkanCommandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
        if (vulkanCommandBuffer)
            vkCommandBuffers.emplace_back(vulkanCommandBuffer->commandBuffer);
    }

    VkFence vkFenceToSignal{ VK_NULL_HANDLE };
    VulkanFence *vulkanFence = vulkanResourceManager->getFence(options.signalFence);
    if (vulkanFence)
        vkFenceToSignal = vulkanFence->fence;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(vkWaitSemaphores.size());
    submitInfo.pWaitSemaphores = vkWaitSemaphores.data();
    submitInfo.pWaitDstStageMask = vkWaitStageFlags.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(vkSignalSemaphores.size());
    submitInfo.pSignalSemaphores = vkSignalSemaphores.data();
    submitInfo.commandBufferCount = vkCommandBuffers.size();
    submitInfo.pCommandBuffers = vkCommandBuffers.data();

#if VK_KHR_timeline_semaphore
    // Chain VkTimelineSemaphoreSubmitInfo when any timeline semaphores are used
    VkTimelineSemaphoreSubmitInfo timelineInfo = {};
    if (waitTimelineCount > 0 || signalTimelineCount > 0) {
        timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.pNext = nullptr;
        timelineInfo.waitSemaphoreValueCount = static_cast<uint32_t>(vkWaitSemaphoreValues.size());
        timelineInfo.pWaitSemaphoreValues = vkWaitSemaphoreValues.data();
        timelineInfo.signalSemaphoreValueCount = static_cast<uint32_t>(vkSignalSemaphoreValues.size());
        timelineInfo.pSignalSemaphoreValues = vkSignalSemaphoreValues.data();
        submitInfo.pNext = &timelineInfo;
    }
#endif

    const VkResult result = vkQueueSubmit(queue, 1, &submitInfo, vkFenceToSignal);
    if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Queue Submission failed {}", result);
    }
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
    std::vector<VkSemaphore> presentVkWaitSemaphores;
    presentVkWaitSemaphores.reserve(waitSemaphoreCount);
    for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.waitSemaphores.at(i));
        if (vulkanSemaphore)
            presentVkWaitSemaphores.push_back(vulkanSemaphore->semaphore);
    }

    const uint32_t swapchainCount = static_cast<uint32_t>(options.swapchainInfos.size());
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<uint32_t> imageIndices;
    swapchains.reserve(swapchainCount);
    imageIndices.reserve(swapchainCount);
    for (uint32_t i = 0; i < swapchainCount; ++i) {
        auto vulkanSwapchain = vulkanResourceManager->getSwapchain(options.swapchainInfos.at(i).swapchain);
        if (vulkanSwapchain) {
            swapchains.push_back(vulkanSwapchain->swapchain);
            imageIndices.push_back(options.swapchainInfos.at(i).imageIndex);
        }
    }

    m_presentResults.clear();
    m_presentResults.resize(swapchains.size());

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(presentVkWaitSemaphores.size());
    presentInfo.pWaitSemaphores = presentVkWaitSemaphores.data();
    presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndices.data();
    presentInfo.pResults = m_presentResults.data();

#if VK_KHR_swapchain_maintenance1
    VkSwapchainPresentFenceInfoKHR presentFenceInfo = {};
    presentFenceInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR;
    presentFenceInfo.swapchainCount = presentInfo.swapchainCount;

    std::vector<VkFence> presentVkFencesToSignal;

    if (options.signalFence.size() > 0) {
        presentVkFencesToSignal.resize(presentFenceInfo.swapchainCount);
        assert(options.signalFence.size() <= presentVkFencesToSignal.size());

        size_t lastFenceIndex = 0;
        for (const auto &fenceHandle : options.signalFence) {
            VulkanFence *vulkanFence = vulkanResourceManager->getFence(fenceHandle);
            if (vulkanFence != nullptr) {
                presentVkFencesToSignal[lastFenceIndex++] = vulkanFence->fence;
            }
        }
        presentFenceInfo.pFences = presentVkFencesToSignal.data();

        // Set VkSwapchainPresentFenceInfoKHR on VkPresentInfoKHR
        presentInfo.pNext = &presentFenceInfo;
    }
#else
    if (!options.signalFence.empty()) {
        // We have fences to signal but the extension isn't available
        SPDLOG_LOGGER_WARN(Logger::logger(), "PresentOptions included signal fences but VK_EXT_swapchain_maintenance1 is not available, ignoring fences");
    }
#endif

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
