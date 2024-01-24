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

#if defined(KDGPU_PLATFORM_WIN32)
// Avoid having to define VK_USE_PLATFORM_WIN32_KHR which would result in windows.h being included when vulkan.h is included
#include <vulkan/vulkan_win32.h>
#endif

namespace KDGpu {

VulkanDevice::VulkanDevice(VkDevice _device,
                           uint32_t _apiVersion,
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
    allocatorInfo.vulkanApiVersion = _apiVersion;
    allocatorInfo.instance = vulkanInstance->instance;
    allocatorInfo.physicalDevice = vulkanAdapter->physicalDevice;
    allocatorInfo.device = device;

    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to create Vulkan memory allocator!");

    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(vulkanAdapter->physicalDevice, &memoryProperties);

    std::vector<VkExternalMemoryHandleTypeFlags> externalMemoryHandleTypes;
#if defined(KDGPU_PLATFORM_LINUX)
    externalMemoryHandleTypes.resize(memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
    allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
#elif defined(KDGPU_PLATFORM_WIN32)
    externalMemoryHandleTypes.resize(memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
    allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
#endif

    if (vmaCreateAllocator(&allocatorInfo, &externalAllocator) != VK_SUCCESS)
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to create Vulkan external memory allocator!");

    // Resize the vector of command pools to have one for each queue family
    const auto queueTypes = vulkanAdapter->queryQueueTypes();
    const auto queueTypeCount = queueTypes.size();
    commandPools.resize(queueTypeCount);
    for (uint32_t i = 0; i < queueTypeCount; ++i)
        commandPools[i] = VK_NULL_HANDLE;

    const auto instanceExtensions = vulkanInstance->extensions();
    for (const auto &extension : instanceExtensions) {
        if (extension.name == "VK_EXT_debug_utils") {
            this->vkSetDebugUtilsObjectNameEXT = PFN_vkSetDebugUtilsObjectNameEXT(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
            break;
        }
    }

#if defined(VK_KHR_synchronization2)
    // Check to see if we have the VK_KHR_synchronization2 extension or not
    if (vulkanAdapter->supportsSynchronization2) {
        const auto adapterExtensions = vulkanAdapter->extensions();
        for (const auto &extension : adapterExtensions) {
            if (extension.name == "VK_KHR_synchronization2") {
                PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR = PFN_vkCmdPipelineBarrier2KHR(
                        vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR"));
                this->vkCmdPipelineBarrier2 = vkCmdPipelineBarrier2KHR;
                break;
            }
        }
    }
#endif

#if defined(KDGPU_PLATFORM_LINUX)
    vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
    vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
#endif

#if defined(KDGPU_PLATFORM_WIN32)
    vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR");
    vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetFenceWin32HandleKHR");
#endif

    // If we request the extension version of renderpass2, then use that. Otherwise fall back to the core 1.2 version.
    const auto adapterExtensions = vulkanAdapter->extensions();
    for (const auto &extension : adapterExtensions) {
        if (extension.name == "VK_KHR_create_renderpass2") {
            this->vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)vkGetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
            break;
        }
    }

    if (this->vkCreateRenderPass2 == nullptr) {
        this->vkCreateRenderPass2 = ::vkCreateRenderPass2;
    }
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
