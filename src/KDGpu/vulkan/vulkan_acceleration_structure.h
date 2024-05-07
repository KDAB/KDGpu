/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_acceleration_structure.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace KDGpu {

class GraphicsApi;
class VulkanResourceManager;

struct Buffer_t;
struct Device_t;
struct AccelerationStructure_t;

/**
 * @brief VulkanAccelerationStructure
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanAccelerationStructure : public ApiAccelerationStructure {
    explicit VulkanAccelerationStructure(Handle<Device_t> _deviceHandle,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         VkAccelerationStructureKHR _accelerationStructure,
                                         Handle<Buffer_t> _backingBuffer,
                                         Handle<Buffer_t> _scratchBuffer,
                                         AccelerationStructureType _type);

    Handle<Device_t> deviceHandle;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    VkAccelerationStructureKHR accelerationStructure{ VK_NULL_HANDLE };
    Handle<Buffer_t> backingBuffer;
    Handle<Buffer_t> scratchBuffer;
    AccelerationStructureType type;
};

} // namespace KDGpu
