/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_command_recorder.h"
#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/vulkan/vulkan_buffer.h>
#include <KDGpu/vulkan/vulkan_command_buffer.h>
#include <KDGpu/vulkan/vulkan_enums.h>

// MemoryBarrier is a define in winnt.h
#if defined(MemoryBarrier)
#undef MemoryBarrier
#endif

namespace {

std::vector<VkBufferImageCopy> buildRegions(const std::vector<KDGpu::BufferTextureCopyRegion> &regions)
{
    const uint32_t regionCount = regions.size();
    std::vector<VkBufferImageCopy> vkRegions;
    vkRegions.reserve(regionCount);
    for (uint32_t i = 0; i < regionCount; ++i) {
        const auto &region = regions.at(i);
        const VkBufferImageCopy vkRegion = {
            .bufferOffset = region.bufferOffset,
            .bufferRowLength = region.bufferRowLength,
            .bufferImageHeight = region.bufferTextureHeight,
            .imageSubresource = {
                    .aspectMask = region.textureSubResource.aspectMask.toInt(),
                    .mipLevel = region.textureSubResource.mipLevel,
                    .baseArrayLayer = region.textureSubResource.baseArrayLayer,
                    .layerCount = region.textureSubResource.layerCount },
            .imageOffset = { .x = region.textureOffset.x, .y = region.textureOffset.y, .z = region.textureOffset.z },
            .imageExtent = { .width = region.textureExtent.width, .height = region.textureExtent.height, .depth = region.textureExtent.depth }
        };
        vkRegions.emplace_back(std::move(vkRegion));
    }
    return vkRegions;
}

std::vector<VkImageCopy> buildRegions(const std::vector<KDGpu::TextureCopyRegion> &regions)
{
    const uint32_t regionCount = regions.size();
    std::vector<VkImageCopy> vkRegions;
    vkRegions.reserve(regionCount);
    for (uint32_t i = 0; i < regionCount; ++i) {
        const auto &region = regions.at(i);
        // clang-format off
        const VkImageCopy vkRegion = {
            .srcSubresource = {
                .aspectMask = region.srcSubresource.aspectMask.toInt(),
                .mipLevel = region.srcSubresource.mipLevel,
                .baseArrayLayer = region.srcSubresource.baseArrayLayer,
                .layerCount = region.srcSubresource.layerCount },
            .srcOffset = { .x = region.srcOffset.x, .y = region.srcOffset.y, .z = region.srcOffset.z },
            .dstSubresource = {
                .aspectMask = region.dstSubresource.aspectMask.toInt(),
                .mipLevel = region.dstSubresource.mipLevel,
                .baseArrayLayer = region.srcSubresource.baseArrayLayer,
                .layerCount = region.dstSubresource.layerCount },
            .dstOffset = { .x = region.dstOffset.x, .y = region.dstOffset.y, .z = region.dstOffset.z },
            .extent = { .width = region.extent.width, .height = region.extent.height, .depth = region.extent.depth }
            // clang-format on
        };
        vkRegions.emplace_back(std::move(vkRegion));
    }
    return vkRegions;
}

} // namespace

namespace KDGpu {

VulkanCommandRecorder::VulkanCommandRecorder(VkCommandPool _commandPool,
                                             const Handle<CommandBuffer_t> _commandBufferHandle,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle)
    : ApiCommandRecorder()
    , commandPool(_commandPool)
    , commandBufferHandle(_commandBufferHandle)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
    VulkanCommandBuffer *vulkanCommandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer = vulkanCommandBuffer->commandBuffer;
}

void VulkanCommandRecorder::begin()
{
    VulkanCommandBuffer *commandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer->begin();
}

void VulkanCommandRecorder::copyBuffer(const BufferCopy &copy)
{
    VulkanBuffer *srcBuf = vulkanResourceManager->getBuffer(copy.src);
    VulkanBuffer *dstBuf = vulkanResourceManager->getBuffer(copy.dst);

    VkBufferCopy bufferCopy{};
    bufferCopy.size = copy.byteSize;
    bufferCopy.dstOffset = copy.dstOffset;
    bufferCopy.srcOffset = copy.srcOffset;

    vkCmdCopyBuffer(commandBuffer, srcBuf->buffer, dstBuf->buffer, 1, &bufferCopy);
}

void VulkanCommandRecorder::copyBufferToTexture(const BufferToTextureCopy &copy)
{
    VulkanBuffer *srcVulkanBuffer = vulkanResourceManager->getBuffer(copy.srcBuffer);
    VulkanTexture *dstVulkanTexture = vulkanResourceManager->getTexture(copy.dstTexture);
    const std::vector<VkBufferImageCopy> vkRegions = buildRegions(copy.regions);

    vkCmdCopyBufferToImage(commandBuffer,
                           srcVulkanBuffer->buffer,
                           dstVulkanTexture->image,
                           textureLayoutToVkImageLayout(copy.dstTextureLayout),
                           static_cast<uint32_t>(vkRegions.size()),
                           vkRegions.data());
}

void VulkanCommandRecorder::copyTextureToBuffer(const TextureToBufferCopy &copy)
{
    VulkanTexture *srcVulkanTexture = vulkanResourceManager->getTexture(copy.srcTexture);
    VulkanBuffer *dstVulkanBuffer = vulkanResourceManager->getBuffer(copy.dstBuffer);
    const std::vector<VkBufferImageCopy> vkRegions = buildRegions(copy.regions);

    vkCmdCopyImageToBuffer(commandBuffer,
                           srcVulkanTexture->image,
                           textureLayoutToVkImageLayout(copy.srcTextureLayout),
                           dstVulkanBuffer->buffer,
                           static_cast<uint32_t>(vkRegions.size()),
                           vkRegions.data());
}

void VulkanCommandRecorder::copyTextureToTexture(const TextureToTextureCopy &copy)
{
    VulkanTexture *srcVulkanTexture = vulkanResourceManager->getTexture(copy.srcTexture);
    VulkanTexture *dstVulkanTexture = vulkanResourceManager->getTexture(copy.dstTexture);
    const std::vector<VkImageCopy> vkRegions = buildRegions(copy.regions);

    vkCmdCopyImage(commandBuffer,
                   srcVulkanTexture->image,
                   textureLayoutToVkImageLayout(copy.srcLayout),
                   dstVulkanTexture->image,
                   textureLayoutToVkImageLayout(copy.dstLayout),
                   static_cast<uint32_t>(vkRegions.size()),
                   vkRegions.data());
}

void VulkanCommandRecorder::memoryBarrier(const MemoryBarrierOptions &options)
{
    std::vector<VkMemoryBarrier> memoryBarriers;

    memoryBarriers.reserve(options.memoryBarriers.size());
    for (const MemoryBarrier &b : options.memoryBarriers) {
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = accessFlagsToVkAccessFlagBits(b.srcMask);
        barrier.dstAccessMask = accessFlagsToVkAccessFlagBits(b.dstMask);
        memoryBarriers.push_back(barrier);
    }

    vkCmdPipelineBarrier(commandBuffer,
                         pipelineStageFlagsToVkPipelineStageFlagBits(options.srcStages),
                         pipelineStageFlagsToVkPipelineStageFlagBits(options.dstStages),
                         0, // None
                         memoryBarriers.size(), memoryBarriers.data(),
                         0, nullptr,
                         0, nullptr);
}

// TODO: Implement an array version. Perhaps also a way to refer to the set of arguments via a
// handle to a backend type if we find we keep issuing barriers in the same way many times.
void VulkanCommandRecorder::bufferMemoryBarrier(const BufferMemoryBarrierOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkCmdPipelineBarrier2 != nullptr) {
        VkBufferMemoryBarrier2KHR vkBufferBarrier = {};
        vkBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR;
        vkBufferBarrier.srcStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.srcStages);
        vkBufferBarrier.srcAccessMask = accessFlagsToVkAccessFlagBits2(options.srcMask);
        vkBufferBarrier.dstStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.dstStages);
        vkBufferBarrier.dstAccessMask = accessFlagsToVkAccessFlagBits2(options.dstMask);
        vkBufferBarrier.srcQueueFamilyIndex = options.srcQueueTypeIndex;
        vkBufferBarrier.dstQueueFamilyIndex = options.dstQueueTypeIndex;

        const auto vulkanBuffer = vulkanResourceManager->getBuffer(options.buffer);
        vkBufferBarrier.buffer = vulkanBuffer->buffer;
        vkBufferBarrier.offset = options.offset;
        vkBufferBarrier.size = options.size;

        VkDependencyInfoKHR vkDependencyInfo = {};
        vkDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        vkDependencyInfo.bufferMemoryBarrierCount = 1;
        vkDependencyInfo.pBufferMemoryBarriers = &vkBufferBarrier;

        vulkanDevice->vkCmdPipelineBarrier2(commandBuffer, &vkDependencyInfo);
    } else {
        // Fallback to the Vulkan 1.0 approach
        VkBufferMemoryBarrier vkBufferBarrier = {};
        vkBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        vkBufferBarrier.srcAccessMask = accessFlagsToVkAccessFlagBits(options.srcMask);
        vkBufferBarrier.dstAccessMask = accessFlagsToVkAccessFlagBits(options.dstMask);

        const auto vulkanBuffer = vulkanResourceManager->getBuffer(options.buffer);
        vkBufferBarrier.buffer = vulkanBuffer->buffer;
        vkBufferBarrier.offset = options.offset;
        vkBufferBarrier.size = options.size;

        vkCmdPipelineBarrier(commandBuffer,
                             pipelineStageFlagsToVkPipelineStageFlagBits(options.srcStages),
                             pipelineStageFlagsToVkPipelineStageFlagBits(options.dstStages),
                             0, // None
                             0, nullptr,
                             1, &vkBufferBarrier,
                             0, nullptr);
    }
}

// TODO: Implement an array version. Perhaps also a way to refer to the set of arguments via a
// handle to a backend type if we find we keep issuing barriers in the same way many times.
void VulkanCommandRecorder::textureMemoryBarrier(const TextureMemoryBarrierOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkCmdPipelineBarrier2 != nullptr) {
        VkImageMemoryBarrier2KHR vkImageBarrier = {};
        vkImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
        vkImageBarrier.srcStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.srcStages);
        vkImageBarrier.srcAccessMask = accessFlagsToVkAccessFlagBits2(options.srcMask);
        vkImageBarrier.dstStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.dstStages);
        vkImageBarrier.dstAccessMask = accessFlagsToVkAccessFlagBits2(options.dstMask);
        vkImageBarrier.srcQueueFamilyIndex = options.srcQueueTypeIndex;
        vkImageBarrier.dstQueueFamilyIndex = options.dstQueueTypeIndex;
        vkImageBarrier.oldLayout = textureLayoutToVkImageLayout(options.oldLayout);
        vkImageBarrier.newLayout = textureLayoutToVkImageLayout(options.newLayout);

        const auto vulkanTexture = vulkanResourceManager->getTexture(options.texture);
        vkImageBarrier.image = vulkanTexture->image;
        vkImageBarrier.subresourceRange = {
            .aspectMask = options.range.aspectMask.toInt(),
            .baseMipLevel = options.range.baseMipLevel,
            .levelCount = options.range.levelCount,
            .baseArrayLayer = options.range.baseArrayLayer,
            .layerCount = options.range.layerCount
        };

        VkDependencyInfoKHR vkDependencyInfo = {};
        vkDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        vkDependencyInfo.imageMemoryBarrierCount = 1;
        vkDependencyInfo.pImageMemoryBarriers = &vkImageBarrier;

        vulkanDevice->vkCmdPipelineBarrier2(commandBuffer, &vkDependencyInfo);
    } else {
        // Fallback to the Vulkan 1.0 approach
        VkImageMemoryBarrier vkImageBarrier = {};
        vkImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vkImageBarrier.srcAccessMask = accessFlagsToVkAccessFlagBits(options.srcMask);
        vkImageBarrier.dstAccessMask = accessFlagsToVkAccessFlagBits(options.dstMask);
        vkImageBarrier.srcQueueFamilyIndex = options.srcQueueTypeIndex;
        vkImageBarrier.dstQueueFamilyIndex = options.dstQueueTypeIndex;
        vkImageBarrier.oldLayout = textureLayoutToVkImageLayout(options.oldLayout);
        vkImageBarrier.newLayout = textureLayoutToVkImageLayout(options.newLayout);

        const auto vulkanTexture = vulkanResourceManager->getTexture(options.texture);
        vkImageBarrier.image = vulkanTexture->image;
        vkImageBarrier.subresourceRange = {
            .aspectMask = options.range.aspectMask.toInt(),
            .baseMipLevel = options.range.baseMipLevel,
            .levelCount = options.range.levelCount,
            .baseArrayLayer = options.range.baseArrayLayer,
            .layerCount = options.range.layerCount
        };

        vkCmdPipelineBarrier(commandBuffer,
                             pipelineStageFlagsToVkPipelineStageFlagBits(options.srcStages),
                             pipelineStageFlagsToVkPipelineStageFlagBits(options.dstStages),
                             0, // None
                             0, nullptr,
                             0, nullptr,
                             1, &vkImageBarrier);
    }
}

void VulkanCommandRecorder::executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer)
{
    VulkanCommandBuffer *vulkanSecondaryCommandBuffer = vulkanResourceManager->getCommandBuffer(secondaryCommandBuffer);
    vkCmdExecuteCommands(commandBuffer, 1, &vulkanSecondaryCommandBuffer->commandBuffer);
}

Handle<CommandBuffer_t> VulkanCommandRecorder::finish()
{
    VulkanCommandBuffer *commandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer->finish();
    return commandBufferHandle;
}

} // namespace KDGpu
