#include "vulkan_queue.h"

#include <toy_renderer/queue.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanQueue::VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager)
    : ApiQueue()
    , queue(_queue)
    , vulkanResourceManager(_vulkanResourceManager)
{
}

void VulkanQueue::submit()
{
    // TODO: Implement me!
}

void VulkanQueue::present(const PresentOptions &options)
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

    // TODO: Map the return code to something api agnostic
}

} // namespace ToyRenderer
