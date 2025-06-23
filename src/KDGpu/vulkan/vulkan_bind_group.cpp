/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_bind_group.h"
#include <KDGpu/bind_group_options.h>
#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_device.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanBindGroup::VulkanBindGroup(VkDescriptorSet _descriptorSet,
                                 const Handle<BindGroupPool_t> &_bindGroupPoolHandle,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle,
                                 bool _implicitFree)
    : descriptorSet(_descriptorSet)
    , bindGroupPoolHandle(_bindGroupPoolHandle)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , implicitFree(_implicitFree)
{
}

void VulkanBindGroup::update(const BindGroupEntry &entry)
{
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    VkDescriptorBufferInfo bufferInfo{};

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (descriptorSet == VK_NULL_HANDLE) {
        // If the descriptor set is null, we cannot update it.
        // This can happen if the pool has been reset and our BindGroup kept alive
        SPDLOG_LOGGER_ERROR(Logger::logger(), "BindGroup Vulkan Handle is NULL, unable to update. This can happen if the BindGroupPool has been reset.");
        return;
    }

    WriteBindGroupData bindGroupWriteData;
    vulkanDevice->fillWriteBindGroupDataForBindGroupEntry(bindGroupWriteData, entry, descriptorSet);

    if (bindGroupWriteData.descriptorWrite.descriptorCount > 0)
        vkUpdateDescriptorSets(vulkanDevice->device, 1, &bindGroupWriteData.descriptorWrite, 0, nullptr);
}

} // namespace KDGpu
