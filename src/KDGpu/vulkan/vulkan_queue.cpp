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
    // TODO: Do we need to expose the wait stage flags to the public API or is waiting
    // for the semaphores at the top of the pipeline good enough?
    const uint32_t waitSemaphoreCount = static_cast<uint32_t>(options.waitSemaphores.size());
    std::vector<VkSemaphore> vkWaitSemaphores;
    std::vector<VkPipelineStageFlags> vkWaitStageFlags;
    vkWaitSemaphores.reserve(waitSemaphoreCount);
    vkWaitStageFlags.reserve(waitSemaphoreCount);
    for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.waitSemaphores.at(i));
        if (vulkanSemaphore) {
            vkWaitSemaphores.push_back(vulkanSemaphore->semaphore);
            vkWaitStageFlags.push_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
    }

    const uint32_t signalSemaphoreCount = static_cast<uint32_t>(options.signalSemaphores.size());
    std::vector<VkSemaphore> vkSignalSemaphores;
    vkSignalSemaphores.reserve(signalSemaphoreCount);
    for (uint32_t i = 0; i < signalSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.signalSemaphores.at(i));
        if (vulkanSemaphore)
            vkSignalSemaphores.push_back(vulkanSemaphore->semaphore);
    }

    const uint32_t commandBufferCount = static_cast<uint32_t>(options.commandBuffers.size());
    std::vector<VkCommandBuffer> vkCommandBuffers;
    vkCommandBuffers.reserve(commandBufferCount);
    for (uint32_t i = 0; i < commandBufferCount; ++i) {
        auto vulkanCommandBuffer = vulkanResourceManager->getCommandBuffer(options.commandBuffers.at(i));
        if (vulkanCommandBuffer)
            vkCommandBuffers.push_back(vulkanCommandBuffer->commandBuffer);
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

    submitInfo.commandBufferCount = static_cast<uint32_t>(vkCommandBuffers.size());
    submitInfo.pCommandBuffers = vkCommandBuffers.data();

    // TODO: Support fences
    // Make sure the fence is ready for use and submit
    // VkFence inFlightFences[] = { frameFence };
    // vkResetFences(renderer()->vulkanDevice()->device(), 1, inFlightFences);

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, vkFenceToSignal);
}

std::vector<PresentResult> VulkanQueue::present(const PresentOptions &options)
{
    const uint32_t waitSemaphoreCount = static_cast<uint32_t>(options.waitSemaphores.size());
    std::vector<VkSemaphore> vkWaitSemaphores;
    vkWaitSemaphores.reserve(waitSemaphoreCount);
    for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
        auto vulkanSemaphore = vulkanResourceManager->getGpuSemaphore(options.waitSemaphores.at(i));
        if (vulkanSemaphore)
            vkWaitSemaphores.push_back(vulkanSemaphore->semaphore);
    }

    const uint32_t swapchainCount = static_cast<uint32_t>(options.swapchainInfos.size());
    std::vector<VkSwapchainKHR> vkSwapchains;
    std::vector<uint32_t> imageIndices;
    vkSwapchains.reserve(swapchainCount);
    imageIndices.reserve(swapchainCount);
    for (uint32_t i = 0; i < swapchainCount; ++i) {
        auto vulkanSwapchain = vulkanResourceManager->getSwapchain(options.swapchainInfos.at(i).swapchain);
        if (vulkanSwapchain) {
            vkSwapchains.push_back(vulkanSwapchain->swapchain);
            imageIndices.push_back(options.swapchainInfos.at(i).imageIndex);
        }
    }
    std::vector<VkResult> presentResults(vkSwapchains.size());

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vkWaitSemaphores.size());
    presentInfo.pWaitSemaphores = vkWaitSemaphores.data();
    presentInfo.swapchainCount = static_cast<uint32_t>(vkSwapchains.size());
    presentInfo.pSwapchains = vkSwapchains.data();
    presentInfo.pImageIndices = imageIndices.data();
    presentInfo.pResults = presentResults.data();

    const VkResult result = vkQueuePresentKHR(queue, &presentInfo);

    // If result is success, then all swapchains were sucessfully presented
    if (result == VK_SUCCESS)
        return std::vector<PresentResult>(swapchainCount, PresentResult::Success);

    std::vector<PresentResult> results;
    results.reserve(swapchainCount);

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

    for (VkResult r : presentResults)
        results.emplace_back(mapVkResultToPresentResult(r));

    return results;
}

} // namespace KDGpu
