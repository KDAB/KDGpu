/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_acceleration_structure.h"

#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/buffer_options.h>

namespace KDGpu {

VulkanAccelerationStructure::VulkanAccelerationStructure(Handle<Device_t> _deviceHandle,
                                                         VulkanResourceManager *_vulkanResourceManager,
                                                         VkAccelerationStructureKHR _accelerationStructure,
                                                         Handle<Buffer_t> _backingBuffer,
                                                         AccelerationStructureType _type,
                                                         VkAccelerationStructureBuildSizesInfoKHR _buildSizes,
                                                         VkBuildAccelerationStructureFlagsKHR _buildFlags)
    : ApiAccelerationStructure()
    , deviceHandle(_deviceHandle)
    , vulkanResourceManager(_vulkanResourceManager)
    , accelerationStructure(_accelerationStructure)
    , backingBuffer(_backingBuffer)
    , type(_type)
    , buildSizes(_buildSizes)
    , buildFlags(_buildFlags)
{
}

Handle<Buffer_t> VulkanAccelerationStructure::createAccelerationBuffer(Handle<Device_t> deviceHandle,
                                                                       VulkanResourceManager *vulkanResourceManager,
                                                                       VkDeviceSize size)
{
    return vulkanResourceManager->createBuffer(deviceHandle, BufferOptions{
                                                                     .size = size,
                                                                     .usage = BufferUsageFlagBits::StorageBufferBit | BufferUsageFlagBits::AccelerationStructureStorageBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
                                                                     .memoryUsage = MemoryUsage::GpuOnly,
                                                             },
                                               nullptr);
}

} // namespace KDGpu
