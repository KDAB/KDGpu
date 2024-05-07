/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_acceleration_structure.h"

#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanAccelerationStructure::VulkanAccelerationStructure(Handle<Device_t> _deviceHandle,
                                                         VulkanResourceManager *_vulkanResourceManager,
                                                         VkAccelerationStructureKHR _accelerationStructure,
                                                         Handle<Buffer_t> _backingBuffer,
                                                         Handle<Buffer_t> _scratchBuffer,
                                                         AccelerationStructureType _type)
    : ApiAccelerationStructure()
    , deviceHandle(_deviceHandle)
    , vulkanResourceManager(_vulkanResourceManager)
    , accelerationStructure(_accelerationStructure)
    , backingBuffer(_backingBuffer)
    , scratchBuffer(_scratchBuffer)
    , type(_type)
{
}

} // namespace KDGpu
