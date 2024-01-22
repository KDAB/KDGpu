/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_graphics_api.h"

#include <KDGpu/vulkan/vulkan_config.h>
#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/texture_options.h>

namespace KDGpu {

std::vector<std::string> VulkanGraphicsApi::ms_ignoredErrors = KDGpu::defaultIgnoredErrors;

VulkanGraphicsApi::VulkanGraphicsApi()
    : GraphicsApi()
    , m_vulkanResourceManager{ std::make_unique<VulkanResourceManager>() }
{
    m_resourceManager = m_vulkanResourceManager.get();
}

VulkanGraphicsApi::~VulkanGraphicsApi()
{
}

Instance VulkanGraphicsApi::createInstanceFromExistingVkInstance(VkInstance vkInstance)
{
    Instance instance;
    instance.m_api = this;
    instance.m_instance = m_vulkanResourceManager->createInstanceFromExistingVkInstance(vkInstance);
    return instance;
}

Surface VulkanGraphicsApi::createSurfaceFromExistingVkSurface(const Handle<Instance_t> &instanceH, VkSurfaceKHR vkSurface)
{
    VulkanInstance *instance = m_vulkanResourceManager->getInstance(instanceH);
    return Surface(this, instance->createSurface(vkSurface));
}

Adapter VulkanGraphicsApi::createAdapterFromExistingVkPhysicalDevice(const Handle<Instance_t> &instanceH, VkPhysicalDevice vkPhysicalDevice)
{
    return Adapter(this,
                   m_vulkanResourceManager->insertAdapter(
                           VulkanAdapter(vkPhysicalDevice, m_vulkanResourceManager.get(), instanceH)));
}

Queue VulkanGraphicsApi::createQueueFromExistingVkQueue(VkQueue vkQueue, const QueueFlags queueFlags)
{
    const Handle<Queue_t> queueHandle = m_vulkanResourceManager->insertQueue(VulkanQueue(vkQueue, m_vulkanResourceManager.get()));
    return Queue(this,
                 {},
                 QueueDescription{
                         .queue = queueHandle,
                         .flags = queueFlags,
                         .queueTypeIndex = 0,
                         // We can't deduce the other Fields
                 });
}

Device VulkanGraphicsApi::createDeviceFromExistingVkDevice(Adapter *adapter,
                                                           VkDevice vkDevice,
                                                           std::vector<Queue> &&queues)
{
    Device device;
    device.m_api = this;
    device.m_adapter = adapter;
    device.m_device = m_vulkanResourceManager->createDeviceFromExistingVkDevice(adapter->handle(), vkDevice);
    device.m_queues = std::move(queues);

    // Note: we can't know what queues the VkDevice was create with, we assume createQueueFromExistingVkQueue
    // will be called by the user to create the queues he needs and passed in to this function

    // Copy the Queue Description into the VulkanDevice as that might be used
    // by the CommandRecorders to resolve which queue to use
    VulkanDevice *vulkanDevice = m_vulkanResourceManager->getDevice(device.m_device);
    assert(vulkanDevice);
    std::vector<QueueDescription> descriptions;
    descriptions.reserve(device.m_queues.size());
    for (Queue &queue : device.m_queues) {
        if (!queue.m_device.isValid())
            queue.m_device = device.m_device; // Set device on queue since we couldn't do it when creating the queue with createQueueFromExistingVkQueue
        descriptions.push_back(QueueDescription{
                .queue = queue.handle(),
                .flags = queue.flags(),
                .timestampValidBits = queue.timestampValidBits(),
                .minImageTransferGranularity = queue.minImageTransferGranularity(),
                .queueTypeIndex = queue.queueTypeIndex(),
        });
    }
    vulkanDevice->queueDescriptions = std::move(descriptions);

    return device;
}

VkImage VulkanGraphicsApi::vkImageFromTexture(const Handle<Texture_t> textureH) const
{
    VulkanTexture *vulkanTexture = m_vulkanResourceManager->getTexture(textureH);
    if (vulkanTexture)
        return vulkanTexture->image;
    return VK_NULL_HANDLE;
}

Texture VulkanGraphicsApi::createTextureFromExistingVkImage(const Handle<Device_t> &deviceHandle, const TextureOptions &options, VkImage vkImage)
{
    return Texture(this, deviceHandle, m_vulkanResourceManager->insertTexture(VulkanTexture(vkImage, VK_NULL_HANDLE, VK_NULL_HANDLE, options.format, options.extent, options.mipLevels, options.arrayLayers, options.usage, m_vulkanResourceManager.get(), deviceHandle, {})));
}

std::string VulkanGraphicsApi::getMemoryStats(const Handle<Device_t> &device) const
{
    return m_vulkanResourceManager->getMemoryStats(device);
}

void VulkanGraphicsApi::addValidationMessageToIgnore(const std::string &messageToIgnore)
{
    ms_ignoredErrors.push_back(messageToIgnore);
}

const std::vector<std::string> &VulkanGraphicsApi::validationMessagesToIgnore()
{
    return ms_ignoredErrors;
}

} // namespace KDGpu
