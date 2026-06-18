/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_dynamic_render_pass_command_recorder.h"

#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanDynamicRenderPassCommandRecorder::VulkanDynamicRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                               VkRect2D _renderArea,
                                                                               VulkanResourceManager *_vulkanResourceManager,
                                                                               const Handle<Device_t> &_deviceHandle)
    : VulkanRenderPassCommandRecorderBase(_commandBuffer, _renderArea, _vulkanResourceManager, _deviceHandle)
{
}

void VulkanDynamicRenderPassCommandRecorder::setOutputAttachmentMapping(std::span<const std::optional<uint32_t>> remappedOutputs) const
{
#if VK_KHR_dynamic_rendering_local_read
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdSetRenderingAttachmentLocationsKHR) {
        std::vector<uint32_t> remappedOutputsOrUnused;
        remappedOutputsOrUnused.reserve(remappedOutputs.size());
        for (const std::optional<uint32_t> &index : remappedOutputs) {
            remappedOutputsOrUnused.push_back(index.value_or(VK_ATTACHMENT_UNUSED));
        }

        VkRenderingAttachmentLocationInfoKHR locationInfo{};
        locationInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR;
        locationInfo.colorAttachmentCount = remappedOutputsOrUnused.size();
        locationInfo.pColorAttachmentLocations = remappedOutputsOrUnused.data();
        device->vkCmdSetRenderingAttachmentLocationsKHR(commandBuffer, &locationInfo);
    }
#else
    assert(false);
#endif
}

void VulkanDynamicRenderPassCommandRecorder::setInputAttachmentMapping(std::span<const std::optional<uint32_t>> colorAttachmentIndices,
                                                                       std::optional<uint32_t> depthAttachmentIndex,
                                                                       std::optional<uint32_t> stencilAttachmentIndex) const
{
#if VK_KHR_dynamic_rendering_local_read
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    if (device->vkCmdSetRenderingInputAttachmentIndicesKHR) {
        std::vector<uint32_t> colorAttachmentIndicesOrUnused;
        colorAttachmentIndicesOrUnused.reserve(colorAttachmentIndices.size());
        for (const std::optional<uint32_t> &index : colorAttachmentIndices) {
            colorAttachmentIndicesOrUnused.push_back(index.value_or(VK_ATTACHMENT_UNUSED));
        }

        const uint32_t depthInputLocationIdx = depthAttachmentIndex.value_or(VK_ATTACHMENT_UNUSED);
        const uint32_t stencilInputLocationIdx = stencilAttachmentIndex.value_or(VK_ATTACHMENT_UNUSED);

        VkRenderingInputAttachmentIndexInfoKHR locationInfo{};
        locationInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR;
        locationInfo.colorAttachmentCount = colorAttachmentIndices.size();
        locationInfo.pColorAttachmentInputIndices = colorAttachmentIndicesOrUnused.data();
        locationInfo.pDepthInputAttachmentIndex = &depthInputLocationIdx;
        locationInfo.pStencilInputAttachmentIndex = &stencilInputLocationIdx;

        device->vkCmdSetRenderingInputAttachmentIndicesKHR(commandBuffer, &locationInfo);
    }
#else
    assert(false);
#endif
}

void VulkanDynamicRenderPassCommandRecorder::end() const
{
#if VK_KHR_dynamic_rendering
    VulkanDevice *device = vulkanResourceManager->getDevice(deviceHandle);
    device->vkCmdEndRenderingKHR(commandBuffer);
#else
    assert(false);
#endif
}

} // namespace KDGpu
