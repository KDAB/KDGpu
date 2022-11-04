#include "vulkan_device.h"

#include <toy_renderer/resource_manager.h>
#include <toy_renderer/vulkan/vulkan_queue.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanDevice::VulkanDevice(VkDevice _device)
    : ApiDevice()
    , device(_device)
{
}

std::vector<QueueDescription> VulkanDevice::getQueues(ResourceManager *resourceManager,
                                                      const std::vector<QueueRequest> &queueRequests,
                                                      std::span<AdapterQueueType> queueTypes)
{
    auto vulkanResourceManager = dynamic_cast<VulkanResourceManager *>(resourceManager);
    assert(vulkanResourceManager);

    uint32_t queueCount = 0;
    for (const auto &queueRequest : queueRequests)
        queueCount += queueRequest.count;
    std::vector<QueueDescription> queueDescriptions;
    queueDescriptions.reserve(queueCount);

    uint32_t i = 0;
    for (const auto &queueRequest : queueRequests) {
        const uint32_t queueCountForFamily = queueRequest.count;
        for (uint32_t j = 0; j < queueCountForFamily; ++j) {
            VkQueue vkQueue{ VK_NULL_HANDLE };
            vkGetDeviceQueue(device, queueRequest.queueTypeIndex, j, &vkQueue);
            const auto queueHandle = vulkanResourceManager->insertQueue(VulkanQueue{ vkQueue });

            QueueDescription queueDescription{
                .queue = queueHandle,
                .flags = queueTypes[queueRequest.queueTypeIndex].flags,
                .timestampValidBits = queueTypes[queueRequest.queueTypeIndex].timestampValidBits,
                .minImageTransferGranularity = queueTypes[queueRequest.queueTypeIndex].minImageTransferGranularity,
                .queueTypeIndex = queueRequest.queueTypeIndex
            };
            queueDescriptions.push_back(queueDescription);

            ++i;
        }
    }

    return queueDescriptions;
}

} // namespace ToyRenderer
