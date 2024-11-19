/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_features.h>
#include <KDGpu/adapter_properties.h>
#include <KDGpu/adapter_queue_type.h>
#include <KDGpu/adapter_swapchain_properties.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>
#include <KDGpu/surface.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanGraphicsApi;
class VulkanResourceManager;

struct Instance_t;

/**
 * @brief VulkanAdapter
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanAdapter {
    explicit VulkanAdapter(VkPhysicalDevice _physicalDevice,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Instance_t> &_instanceHandle);

    std::vector<Extension> extensions() const;
    AdapterProperties queryAdapterProperties();
    AdapterFeatures queryAdapterFeatures();
    AdapterSwapchainProperties querySwapchainProperties(const Handle<Surface_t> &surfaceHandle);
    std::vector<AdapterQueueType> queryQueueTypes();
    bool supportsPresentation(const Handle<Surface_t> surfaceHandle, uint32_t queueTypeIndex);
    FormatProperties formatProperties(Format format) const;

    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Instance_t> instanceHandle;
    std::vector<AdapterQueueType> queueTypes;
    bool supportsSynchronization2{ false };
};

} // namespace KDGpu
