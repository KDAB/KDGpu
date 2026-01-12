/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_type.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

/**
 * @defgroup vulkan Vulkan
 *
 */

/**
 * @page Vulkan
 *
 */

/**
 * @brief VulkanGraphicsApi
 * \ingroup vulkan
 * \ingroup public
 *
 */
class KDGPU_EXPORT VulkanGraphicsApi
{
public:
    VulkanGraphicsApi();
    ~VulkanGraphicsApi();

    ApiType api() { return ApiType::Vulkan; }

    std::string apiName() { return "Vulkan"; }

    /**
     * @brief Create an Instance object given the InstanceOptions @a options
     */
    Instance createInstance(const InstanceOptions &options = InstanceOptions());

    /**
     * @brief Returns the ResourceManager instance for the GraphicsApi
     */
    VulkanResourceManager *resourceManager() noexcept { return &m_vulkanResourceManager; }
    const VulkanResourceManager *resourceManager() const noexcept { return &m_vulkanResourceManager; }

    Instance createInstanceFromExistingVkInstance(VkInstance vkInstance);
    Surface createSurfaceFromExistingVkSurface(const Handle<Instance_t> &instanceH, VkSurfaceKHR vkSurface);
    Adapter createAdapterFromExistingVkPhysicalDevice(const Handle<Instance_t> &instanceH, VkPhysicalDevice vkPhysicalDevice);
    Queue createQueueFromExistingVkQueue(VkQueue vkQueue, const QueueFlags queueFlags);
    Device createDeviceFromExistingVkDevice(Adapter *adapter,
                                            VkDevice vkDevice,
                                            std::vector<Queue> &&queues);
    VkInstance vkInstanceFromInstance(const Handle<Instance_t> &instanceH) const;
    VkPhysicalDevice vkPhysicalDeviceFromAdapter(const Handle<Adapter_t> &adapterH) const;
    VkDevice vkDeviceFromDevice(const Handle<Device_t> &deviceH) const;
    VkImage vkImageFromTexture(const Handle<Texture_t> textureH) const;
    VkQueue vkQueueFromQueue(const Handle<Queue_t> &queueH) const;
    Texture createTextureFromExistingVkImage(const Handle<Device_t> &deviceHandle, const TextureOptions &options, VkImage vkImage);

    std::string getMemoryStats(const Handle<Device_t> &device) const;

    static void addValidationMessageToIgnore(const std::string &messageToIgnore);
    static const std::vector<std::string> &validationMessagesToIgnore();

private:
    VulkanResourceManager m_vulkanResourceManager;
    static std::vector<std::string> ms_ignoredErrors;
};

} // namespace KDGpu
