/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <KDGpu/gpu_core.h>

namespace KDGpu {

class VulkanGraphicsApi;
class VulkanResourceManager;

struct Buffer_t;
struct Device_t;
struct AccelerationStructure_t;

/**
 * @brief VulkanAccelerationStructure
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanAccelerationStructure {
    explicit VulkanAccelerationStructure(Handle<Device_t> _deviceHandle,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         VkAccelerationStructureKHR _accelerationStructure,
                                         Handle<Buffer_t> _backingBuffer,
                                         AccelerationStructureType _type,
                                         VkAccelerationStructureBuildSizesInfoKHR _buildSizes,
                                         VkBuildAccelerationStructureFlagsKHR _buildFlags);

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Handle<Device_t> deviceHandle;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    VkAccelerationStructureKHR accelerationStructure{ VK_NULL_HANDLE };
    Handle<Buffer_t> backingBuffer;
    AccelerationStructureType type;
    VkAccelerationStructureBuildSizesInfoKHR buildSizes;
    VkBuildAccelerationStructureFlagsKHR buildFlags;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    static Handle<Buffer_t> createAccelerationBuffer(Handle<Device_t> deviceHandle, VulkanResourceManager *_vulkanResourceManager, VkDeviceSize size);
};

} // namespace KDGpu
