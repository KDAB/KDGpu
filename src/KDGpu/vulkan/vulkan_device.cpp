/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_device.h"

#include <KDGpu/resource_manager.h>
#include <KDGpu/vulkan/vulkan_queue.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <stdexcept>

namespace KDGpu {

VulkanDevice::VulkanDevice(VkDevice _device,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Adapter_t> &_adapterHandle,
                           bool _isOwned) noexcept
    : ApiDevice()
    , device(_device)
    , vulkanResourceManager(_vulkanResourceManager)
    , adapterHandle(_adapterHandle)
    , isOwned(_isOwned)
{
    // Create an allocator for the device
    VulkanAdapter *vulkanAdapter = vulkanResourceManager->getAdapter(adapterHandle);
    VulkanInstance *vulkanInstance = vulkanResourceManager->getInstance(vulkanAdapter->instanceHandle);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorInfo.instance = vulkanInstance->instance;
    allocatorInfo.physicalDevice = vulkanAdapter->physicalDevice;
    allocatorInfo.device = device;

    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to create Vulkan memory allocator!");

    // Resize the vector of command pools to have one for each queue family
    const auto queueTypes = vulkanAdapter->queryQueueTypes();
    const auto queueTypeCount = queueTypes.size();
    commandPools.resize(queueTypeCount);
    for (uint32_t i = 0; i < queueTypeCount; ++i)
        commandPools[i] = VK_NULL_HANDLE;

#if defined(VK_KHR_synchronization2)
    // Check to see if we have the VK_KHR_synchronization2 extension or not
    const auto adapterExtensions = vulkanAdapter->extensions();
    for (const auto &extension : adapterExtensions) {
        if (extension.name == "VK_KHR_synchronization2") {
            PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR = PFN_vkCmdPipelineBarrier2KHR(
                    vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR"));
            this->vkCmdPipelineBarrier2 = vkCmdPipelineBarrier2KHR;
            break;
        }
    }
#endif
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

    queueDescriptions.clear();
    queueDescriptions.reserve(queueCount);

    uint32_t i = 0;
    for (const auto &queueRequest : queueRequests) {
        const uint32_t queueCountForFamily = queueRequest.count;
        for (uint32_t j = 0; j < queueCountForFamily; ++j) {
            VkQueue vkQueue{ VK_NULL_HANDLE };
            vkGetDeviceQueue(device, queueRequest.queueTypeIndex, j, &vkQueue);
            const auto queueHandle = vulkanResourceManager->insertQueue(VulkanQueue{ vkQueue, vulkanResourceManager });

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

void VulkanDevice::waitUntilIdle()
{
    vkDeviceWaitIdle(device);
}

} // namespace KDGpu
