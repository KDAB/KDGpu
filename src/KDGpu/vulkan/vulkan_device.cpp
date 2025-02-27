/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_device.h"

#include <KDGpu/resource_manager.h>
#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_queue.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <stdexcept>

#if defined(KDGPU_PLATFORM_WIN32)
// Avoid having to define VK_USE_PLATFORM_WIN32_KHR which would result in windows.h being included when vulkan.h is included
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace KDGpu {

VulkanDevice::VulkanDevice(VkDevice _device,
                           uint32_t _apiVersion,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Adapter_t> &_adapterHandle,
                           const AdapterFeatures &_requestedFeatures,
                           bool _isOwned) noexcept
    : device(_device)
    , apiVersion(_apiVersion)
    , requestedFeatures(_requestedFeatures)
    , vulkanResourceManager(_vulkanResourceManager)
    , adapterHandle(_adapterHandle)
    , isOwned(_isOwned)
{
    VulkanAdapter *vulkanAdapter = vulkanResourceManager->getAdapter(adapterHandle);
    VulkanInstance *vulkanInstance = vulkanResourceManager->getInstance(vulkanAdapter->instanceHandle);

    // Create an allocator for the device
    allocator = createMemoryAllocator();

    // Resize the vector of command pools to have one for each queue family
    const auto queueTypes = vulkanAdapter->queryQueueTypes();
    const auto queueTypeCount = queueTypes.size();
    commandPools.resize(queueTypeCount);
    for (uint32_t i = 0; i < queueTypeCount; ++i)
        commandPools[i] = VK_NULL_HANDLE;

#if defined(VK_EXT_debug_utils)
    const auto instanceExtensions = vulkanInstance->extensions();
    for (const auto &extension : instanceExtensions) {
        if (extension.name == VK_EXT_DEBUG_UTILS_EXTENSION_NAME) {
            this->vkSetDebugUtilsObjectNameEXT = PFN_vkSetDebugUtilsObjectNameEXT(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
            this->vkCmdBeginDebugUtilsLabelEXT = PFN_vkCmdBeginDebugUtilsLabelEXT(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
            this->vkCmdEndDebugUtilsLabelEXT = PFN_vkCmdEndDebugUtilsLabelEXT(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
            break;
        }
    }
#endif

#if defined(VK_KHR_synchronization2)
    // Check to see if we have the VK_KHR_synchronization2 extension or not
    if (vulkanAdapter->supportsSynchronization2) {
        const auto adapterExtensions = vulkanAdapter->extensions();
        for (const auto &extension : adapterExtensions) {
            if (extension.name == VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) {
                PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR = PFN_vkCmdPipelineBarrier2KHR(
                        vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR"));
                this->vkCmdPipelineBarrier2 = vkCmdPipelineBarrier2KHR;
                break;
            }
        }
    }
#endif

#if defined(VK_KHR_acceleration_structure)
    if (vulkanAdapter->queryAdapterFeatures().accelerationStructures) {
        const auto adapterExtensions = vulkanAdapter->extensions();
        for (const auto &extension : adapterExtensions) {
            if (extension.name == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) {
                PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = PFN_vkCmdBuildAccelerationStructuresKHR(
                        vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
                this->vkCmdBuildAccelerationStructuresKHR = vkCmdBuildAccelerationStructuresKHR;

                PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = PFN_vkGetAccelerationStructureDeviceAddressKHR(
                        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
                this->vkGetAccelerationStructureDeviceAddressKHR = vkGetAccelerationStructureDeviceAddressKHR;

                PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = PFN_vkCreateAccelerationStructureKHR(
                        vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
                this->vkCreateAccelerationStructureKHR = vkCreateAccelerationStructureKHR;
                PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = PFN_vkDestroyAccelerationStructureKHR(
                        vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
                this->vkDestroyAccelerationStructureKHR = vkDestroyAccelerationStructureKHR;

                PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = PFN_vkGetAccelerationStructureBuildSizesKHR(
                        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
                this->vkGetAccelerationStructureBuildSizesKHR = vkGetAccelerationStructureBuildSizesKHR;
                break;
            }
        }
    }
#endif

#if defined(VK_KHR_ray_tracing_pipeline)
    if (vulkanAdapter->queryAdapterFeatures().rayTracingPipeline) {
        const auto adapterExtensions = vulkanAdapter->extensions();
        for (const auto &extension : adapterExtensions) {
            if (extension.name == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) {
                PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = PFN_vkCreateRayTracingPipelinesKHR(
                        vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
                this->vkCreateRayTracingPipelinesKHR = vkCreateRayTracingPipelinesKHR;

                PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = PFN_vkCmdTraceRaysKHR(
                        vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
                this->vkCmdTraceRaysKHR = vkCmdTraceRaysKHR;

                PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = PFN_vkGetRayTracingShaderGroupHandlesKHR(
                        vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
                this->vkGetRayTracingShaderGroupHandlesKHR = vkGetRayTracingShaderGroupHandlesKHR;
                break;
            }
        }
    }
#endif

#if defined(VK_EXT_mesh_shader)
    if (vulkanAdapter->queryAdapterFeatures().taskShader && vulkanAdapter->queryAdapterFeatures().meshShader) {
        const auto adapterExtensions = vulkanAdapter->extensions();
        for (const auto &extension : adapterExtensions) {
            if (extension.name == VK_EXT_MESH_SHADER_EXTENSION_NAME) {
                PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT = PFN_vkCmdDrawMeshTasksEXT(vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT"));
                this->vkCmdDrawMeshTasksEXT = vkCmdDrawMeshTasksEXT;
                PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT = PFN_vkCmdDrawMeshTasksIndirectEXT(vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectEXT"));
                this->vkCmdDrawMeshTasksIndirectEXT = vkCmdDrawMeshTasksIndirectEXT;
                break;
            }
        }
    }
#endif

#if defined(VK_KHR_external_semaphore_fd)
    vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
#endif

#if defined(VK_KHR_external_fence_fd)
    vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
#endif

#if defined(VK_KHR_external_semaphore_win32)
    vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR");
#endif

#if defined(VK_KHR_external_fence_win32)
    vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetFenceWin32HandleKHR");
#endif

    // If we request the extension version of renderpass2, then use that. Otherwise fall back to the core 1.2 version.
    const auto adapterExtensions = vulkanAdapter->extensions();
    for (const auto &extension : adapterExtensions) {
        if (extension.name == VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) {
            this->vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)vkGetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
            break;
        }
    }

    if (this->vkCreateRenderPass2 == nullptr) {
        this->vkCreateRenderPass2 = ::vkCreateRenderPass2;
    }

#if defined(VK_EXT_host_image_copy)
    if (vulkanAdapter->queryAdapterFeatures().hostImageCopy) {
        const auto adapterExtensions = vulkanAdapter->extensions();
        for (const auto &extension : adapterExtensions) {
            if (extension.name == VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME) {
                this->vkTransitionImageLayout = (PFN_vkTransitionImageLayoutEXT)vkGetDeviceProcAddr(device, "vkTransitionImageLayoutEXT");
                this->vkCopyImageToMemory = (PFN_vkCopyImageToMemoryEXT)vkGetDeviceProcAddr(device, "vkCopyImageToMemoryEXT");
                this->vkCopyMemoryToImage = (PFN_vkCopyMemoryToImageEXT)vkGetDeviceProcAddr(device, "vkCopyMemoryToImageEXT");
                this->vkCopyImageToImage = (PFN_vkCopyImageToImageEXT)vkGetDeviceProcAddr(device, "vkCopyImageToImageEXT");
            }
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

VmaAllocator VulkanDevice::getOrCreateExternalMemoryAllocator(ExternalMemoryHandleTypeFlags externalMemoryHandleType)
{
    VmaAllocator allocator = VK_NULL_HANDLE;
    auto it = std::find_if(externalAllocators.begin(), externalAllocators.end(), [externalMemoryHandleType](const auto &typeAndAllocator) {
        return (typeAndAllocator.externalMemoryHandleType & externalMemoryHandleType) == externalMemoryHandleType;
    });
    if (it == externalAllocators.end()) {
        allocator = createMemoryAllocator(externalMemoryHandleType);
        externalAllocators.emplace_back(externalMemoryHandleType, allocator);
    } else {
        allocator = it->allocator;
    }
    return allocator;
}

VmaAllocator VulkanDevice::createMemoryAllocator(ExternalMemoryHandleTypeFlags externalMemoryHandleType) const
{
    VulkanAdapter *vulkanAdapter = vulkanResourceManager->getAdapter(adapterHandle);
    VulkanInstance *vulkanInstance = vulkanResourceManager->getInstance(vulkanAdapter->instanceHandle);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = apiVersion;
    allocatorInfo.instance = vulkanInstance->instance;
    allocatorInfo.physicalDevice = vulkanAdapter->physicalDevice;
    allocatorInfo.device = device;
    if (requestedFeatures.bufferDeviceAddress)
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    std::vector<VkExternalMemoryHandleTypeFlags> externalMemoryHandleTypes;
    if (externalMemoryHandleType != ExternalMemoryHandleTypeFlagBits::None) {
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(vulkanAdapter->physicalDevice, &memoryProperties);
        externalMemoryHandleTypes.resize(memoryProperties.memoryTypeCount, externalMemoryHandleTypeToVkExternalMemoryHandleType(externalMemoryHandleType));
        allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
    }

    VmaAllocator allocator = VK_NULL_HANDLE;
    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS)
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to create Vulkan external memory allocator!");

    return allocator;
}

} // namespace KDGpu
