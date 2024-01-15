/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/graphics_api.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <memory>

namespace KDGpu {

/**
 * @defgroup vulkan Vulkan
 *
 * Holds the Vulkan implementation of the Rendering API Interfaces
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
class KDGPU_EXPORT VulkanGraphicsApi final : public GraphicsApi
{
public:
    VulkanGraphicsApi();
    ~VulkanGraphicsApi() final;

    const char *apiName() const noexcept final;

    Instance createInstanceFromExistingVkInstance(VkInstance vkInstance);
    Surface createSurfaceFromExistingVkSurface(const Handle<Instance_t> &instanceH, VkSurfaceKHR vkSurface);
    Adapter createAdapterFromExistingVkPhysicalDevice(const Handle<Instance_t> &instanceH, VkPhysicalDevice vkPhysicalDevice);
    Queue createQueueFromExistingVkQueue(VkQueue vkQueue, const QueueFlags queueFlags);
    Device createDeviceFromExistingVkDevice(Adapter *adapter,
                                            VkDevice vkDevice,
                                            std::vector<Queue> &&queues);
    VkImage vkImageFromTexture(const Handle<Texture_t> textureH) const;
    Texture createTextureFromExistingVkImage(const Handle<Device_t> &deviceHandle, const TextureOptions &options, VkImage vkImage);

    std::string getMemoryStats(const Handle<Device_t> &device) const;

private:
    std::unique_ptr<VulkanResourceManager> m_vulkanResourceManager;
};

} // namespace KDGpu
