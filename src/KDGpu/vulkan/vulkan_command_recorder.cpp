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
#include <KDGpu/buffer_options.h>

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

std::vector<VkImageBlit> buildRegions(const std::vector<KDGpu::TextureBlitRegion> &regions)
{
    const uint32_t regionCount = regions.size();
    std::vector<VkImageBlit> vkRegions;
    vkRegions.reserve(regionCount);
    for (uint32_t i = 0; i < regionCount; ++i) {
        const auto &region = regions.at(i);
        const VkImageBlit vkRegion = {
            .srcSubresource = {
                    .aspectMask = region.srcSubresource.aspectMask.toInt(),
                    .mipLevel = region.srcSubresource.mipLevel,
                    .baseArrayLayer = region.srcSubresource.baseArrayLayer,
                    .layerCount = region.srcSubresource.layerCount,
            },
            .srcOffsets = {
                    {
                            .x = region.srcOffset.x,
                            .y = region.srcOffset.y,
                            .z = region.srcOffset.z,
                    },
                    {
                            .x = static_cast<int32_t>(region.srcExtent.width),
                            .y = static_cast<int32_t>(region.srcExtent.height),
                            .z = static_cast<int32_t>(region.srcExtent.depth),
                    },
            },
            .dstSubresource = {
                    .aspectMask = region.dstSubresource.aspectMask.toInt(),
                    .mipLevel = region.dstSubresource.mipLevel,
                    .baseArrayLayer = region.srcSubresource.baseArrayLayer,
                    .layerCount = region.dstSubresource.layerCount,
            },
            .dstOffsets = {
                    {
                            .x = region.dstOffset.x,
                            .y = region.dstOffset.y,
                            .z = region.dstOffset.z,
                    },
                    {
                            .x = static_cast<int32_t>(region.dstExtent.width),
                            .y = static_cast<int32_t>(region.dstExtent.height),
                            .z = static_cast<int32_t>(region.dstExtent.depth),
                    },
            },
        };
        vkRegions.emplace_back(std::move(vkRegion));
    }
    return vkRegions;
}

std::vector<VkImageResolve> buildResolveRegions(const std::vector<KDGpu::TextureResolveRegion> &regions)
{
    const uint32_t regionCount = regions.size();
    std::vector<VkImageResolve> vkRegions;
    vkRegions.reserve(regionCount);
    for (uint32_t i = 0; i < regionCount; ++i) {
        const auto &region = regions.at(i);
        const VkImageResolve vkRegion = {
            .srcSubresource = {
                    .aspectMask = region.srcSubresource.aspectMask.toInt(),
                    .mipLevel = region.srcSubresource.mipLevel,
                    .baseArrayLayer = region.srcSubresource.baseArrayLayer,
                    .layerCount = region.srcSubresource.layerCount,
            },
            .srcOffset = {
                    .x = region.srcOffset.x,
                    .y = region.srcOffset.y,
                    .z = region.srcOffset.z,
            },
            .dstSubresource = {
                    .aspectMask = region.dstSubresource.aspectMask.toInt(),
                    .mipLevel = region.dstSubresource.mipLevel,
                    .baseArrayLayer = region.srcSubresource.baseArrayLayer,
                    .layerCount = region.dstSubresource.layerCount,
            },
            .dstOffset = {
                    .x = region.dstOffset.x,
                    .y = region.dstOffset.y,
                    .z = region.dstOffset.z,
            },
            .extent = {
                    .width = region.extent.width,
                    .height = region.extent.height,
                    .depth = region.extent.depth,
            }
        };
        vkRegions.emplace_back(std::move(vkRegion));
    }
    return vkRegions;
}

std::vector<VkImageSubresourceRange> buildImageSubresourceRanges(const std::vector<KDGpu::TextureSubresourceRange> &ranges)
{
    const uint32_t rangesCount = ranges.size();
    std::vector<VkImageSubresourceRange> vkRanges;
    vkRanges.reserve(rangesCount);

    for (const KDGpu::TextureSubresourceRange &range : ranges) {
        VkImageSubresourceRange vkRange{
            .aspectMask = range.aspectMask.toInt(),
            .baseMipLevel = range.baseMipLevel,
            .levelCount = range.levelCount,
            .baseArrayLayer = range.baseArrayLayer,
            .layerCount = range.layerCount
        };

        vkRanges.emplace_back(std::move(vkRange));
    }

    return vkRanges;
}

template<class>
inline constexpr bool always_false = false;

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

void VulkanCommandRecorder::blitTexture(const TextureBlitOptions &options)
{
    VulkanTexture *srcVulkanTexture = vulkanResourceManager->getTexture(options.srcTexture);
    VulkanTexture *dstVulkanTexture = vulkanResourceManager->getTexture(options.dstTexture);
    const std::vector<VkImageBlit> vkRegions = buildRegions(options.regions);

    vkCmdBlitImage(commandBuffer,
                   srcVulkanTexture->image,
                   textureLayoutToVkImageLayout(options.srcLayout),
                   dstVulkanTexture->image,
                   textureLayoutToVkImageLayout(options.dstLayout),
                   static_cast<uint32_t>(vkRegions.size()),
                   vkRegions.data(),
                   filterModeToVkFilterMode(options.scalingFilter));
}

void VulkanCommandRecorder::clearBuffer(const BufferClear &clear)
{
    VulkanBuffer *dstBuf = vulkanResourceManager->getBuffer(clear.dstBuffer);

    vkCmdFillBuffer(commandBuffer,
                    dstBuf->buffer,
                    clear.dstOffset,
                    clear.byteSize, clear.clearValue);
}

void VulkanCommandRecorder::clearColorTexture(const ClearColorTexture &clear)
{
    VulkanTexture *texture = vulkanResourceManager->getTexture(clear.texture);
    VkClearColorValue clearValue{};
    std::memcpy(&clearValue.uint32[0], &clear.clearValue.uint32[0], 4 * sizeof(uint32_t));
    const std::vector<VkImageSubresourceRange> vkRanges = buildImageSubresourceRanges(clear.ranges);

    vkCmdClearColorImage(commandBuffer,
                         texture->image,
                         textureLayoutToVkImageLayout(clear.layout),
                         &clearValue,
                         vkRanges.size(),
                         vkRanges.data());
}

void VulkanCommandRecorder::clearDepthStencilTexture(const ClearDepthStencilTexture &clear)
{
    VulkanTexture *texture = vulkanResourceManager->getTexture(clear.texture);
    const VkClearDepthStencilValue clearValue{
        .depth = clear.depthClearValue,
        .stencil = clear.stencilClearValue,
    };
    const std::vector<VkImageSubresourceRange> vkRanges = buildImageSubresourceRanges(clear.ranges);

    vkCmdClearDepthStencilImage(commandBuffer,
                                texture->image,
                                textureLayoutToVkImageLayout(clear.layout),
                                &clearValue,
                                vkRanges.size(),
                                vkRanges.data());
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

void VulkanCommandRecorder::updateBuffer(const BufferUpdate &update)
{
    VulkanBuffer *dstVulkanBuffer = vulkanResourceManager->getBuffer(update.dstBuffer);
    // Note: to be used for update size smaller than 65536, we won't warn but Validation Layer should
    vkCmdUpdateBuffer(commandBuffer,
                      dstVulkanBuffer->buffer,
                      update.dstOffset,
                      update.byteSize,
                      update.data);
}

void VulkanCommandRecorder::memoryBarrier(const MemoryBarrierOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

#if defined(VK_KHR_synchronization2)
    if (vulkanDevice->vkCmdPipelineBarrier2 != nullptr) {
        std::vector<VkMemoryBarrier2KHR> memoryBarriers;
        memoryBarriers.reserve(options.memoryBarriers.size());
        for (const MemoryBarrier &b : options.memoryBarriers) {
            VkMemoryBarrier2KHR barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
            barrier.srcStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.srcStages);
            barrier.srcAccessMask = accessFlagsToVkAccessFlagBits2(b.srcMask);
            barrier.dstStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.dstStages);
            barrier.dstAccessMask = accessFlagsToVkAccessFlagBits2(b.dstMask);
            memoryBarriers.push_back(barrier);
        }

        VkDependencyInfoKHR vkDependencyInfo = {};
        vkDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
        vkDependencyInfo.memoryBarrierCount = memoryBarriers.size();
        vkDependencyInfo.pMemoryBarriers = memoryBarriers.data();

        vulkanDevice->vkCmdPipelineBarrier2(commandBuffer, &vkDependencyInfo);
    } else {
#endif
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
#if defined(VK_KHR_synchronization2)
    }
#endif
}

// TODO: Implement an array version. Perhaps also a way to refer to the set of arguments via a
// handle to a backend type if we find we keep issuing barriers in the same way many times.
void VulkanCommandRecorder::bufferMemoryBarrier(const BufferMemoryBarrierOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
#if defined(VK_KHR_synchronization2)
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
#endif
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
#if defined(VK_KHR_synchronization2)
    }
#endif
}

// TODO: Implement an array version. Perhaps also a way to refer to the set of arguments via a
// handle to a backend type if we find we keep issuing barriers in the same way many times.
void VulkanCommandRecorder::textureMemoryBarrier(const TextureMemoryBarrierOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
#if defined(VK_KHR_synchronization2)
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
#endif
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
#if defined(VK_KHR_synchronization2)
    }
#endif
}

void VulkanCommandRecorder::executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer)
{
    VulkanCommandBuffer *vulkanSecondaryCommandBuffer = vulkanResourceManager->getCommandBuffer(secondaryCommandBuffer);
    assert(vulkanSecondaryCommandBuffer->commandLevel == VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkCmdExecuteCommands(commandBuffer, 1, &vulkanSecondaryCommandBuffer->commandBuffer);
}

void VulkanCommandRecorder::resolveTexture(const TextureResolveOptions &options)
{
    VulkanTexture *srcVulkanTexture = vulkanResourceManager->getTexture(options.srcTexture);
    VulkanTexture *dstVulkanTexture = vulkanResourceManager->getTexture(options.dstTexture);
    const std::vector<VkImageResolve> vkRegions = buildResolveRegions(options.regions);

    vkCmdResolveImage(commandBuffer,
                      srcVulkanTexture->image,
                      textureLayoutToVkImageLayout(options.srcLayout),
                      dstVulkanTexture->image,
                      textureLayoutToVkImageLayout(options.dstLayout),
                      static_cast<uint32_t>(vkRegions.size()),
                      vkRegions.data());
}

void VulkanCommandRecorder::buildAccelerationStructures(const BuildAccelerationStructureOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);

    assert(options.buildGeometryInfos.size() == options.buildRangeInfos.size());

    // So it doesn't go out of scope and destroy itself before the cmd is called
    std::vector<std::vector<VkAccelerationStructureGeometryKHR>> geometriesBacking;
    geometriesBacking.reserve(options.buildGeometryInfos.size());

    auto storeTemporaryBuffer = [this](Handle<Buffer_t> bufferH) {
        // Store Buffer into CommandBuffer so that it gets destroyed when the CommandBuffer gets destroyed
        // which is after the command has completed executed
        VulkanCommandBuffer *commandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
        commandBuffer->temporaryBuffersToRelease.push_back(bufferH);
    };

    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> infos;
    std::vector<std::vector<VkAccelerationStructureBuildRangeInfoKHR>> ranges;

    for (const auto &geometryBuildInfo : options.buildGeometryInfos) {
        std::vector<VkAccelerationStructureGeometryKHR> &geometries = geometriesBacking.emplace_back();

        for (const auto &geometry : geometryBuildInfo.geometries) {
            VkAccelerationStructureGeometryKHR geometryKhr = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };

            std::visit([this, vulkanDevice, &geometryKhr, &storeTemporaryBuffer](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, AccelerationStructureGeometryTrianglesData>) {
                    VkAccelerationStructureGeometryTrianglesDataKHR trianglesDataKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
                    trianglesDataKhr.vertexFormat = formatToVkFormat(arg.vertexFormat);
                    trianglesDataKhr.vertexStride = arg.vertexStride;
                    trianglesDataKhr.maxVertex = arg.maxVertex;
                    trianglesDataKhr.indexType = indexTypeToVkIndexType(arg.indexType);

                    if (arg.vertexData.isValid()) {
                        VulkanBuffer *vertexBuffer = vulkanResourceManager->getBuffer(arg.vertexData);
                        trianglesDataKhr.vertexData.deviceAddress = vertexBuffer->bufferDeviceAddress() + arg.vertexDataOffset;
                    }

                    if (arg.indexData.isValid()) {
                        VulkanBuffer *indexBuffer = vulkanResourceManager->getBuffer(arg.indexData);
                        trianglesDataKhr.indexData.deviceAddress = indexBuffer->bufferDeviceAddress() + arg.indexDataOffset;
                    }

                    if (arg.transformData.isValid()) {
                        VulkanBuffer *transformBuffer = vulkanResourceManager->getBuffer(arg.transformData);
                        trianglesDataKhr.transformData.deviceAddress = transformBuffer->bufferDeviceAddress() + arg.transformDataOffset;
                    }

                    geometryKhr.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                    geometryKhr.geometry.triangles = trianglesDataKhr;
                } else if constexpr (std::is_same_v<T, AccelerationStructureGeometryInstancesData>) {

                    std::vector<VkAccelerationStructureInstanceKHR> array;
                    array.reserve(arg.data.size());
                    for (const auto &element : arg.data) {
                        auto structure = vulkanResourceManager->getAccelerationStructure(element.accelerationStructure);

                        VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfoKhr = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
                        deviceAddressInfoKhr.accelerationStructure = structure->accelerationStructure;

                        VkAccelerationStructureInstanceKHR instanceKhr{
                            .instanceCustomIndex = element.instanceCustomIndex,
                            .mask = element.mask,
                            .instanceShaderBindingTableRecordOffset = element.instanceShaderBindingTableRecordOffset,
                            .flags = geometryInstanceFlagsToVkGeometryInstanceFlags(element.flags),
                            .accelerationStructureReference = vulkanDevice->vkGetAccelerationStructureDeviceAddressKHR(vulkanDevice->device, &deviceAddressInfoKhr)
                        };

                        memcpy(instanceKhr.transform.matrix, element.transform, sizeof(float) * 4 * 3);

                        array.push_back(instanceKhr);
                    }

                    Handle<Buffer_t> instanceDataBufferH = vulkanResourceManager->createBuffer(deviceHandle,
                                                                                               BufferOptions{
                                                                                                       .size = array.size() * sizeof(VkAccelerationStructureInstanceKHR),
                                                                                                       .usage = BufferUsageFlagBits::StorageBufferBit | BufferUsageFlagBits::AccelerationStructureBuildInputReadOnlyBit | BufferUsageFlagBits::ShaderDeviceAddressBit,
                                                                                                       .memoryUsage = MemoryUsage::CpuToGpu,
                                                                                               },
                                                                                               array.data());

                    VulkanBuffer *instanceDataBuffer = vulkanResourceManager->getBuffer(instanceDataBufferH);

                    VkAccelerationStructureGeometryInstancesDataKHR instancesDataKhr;
                    instancesDataKhr.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
                    instancesDataKhr.arrayOfPointers = false;
                    instancesDataKhr.data.deviceAddress = instanceDataBuffer->bufferDeviceAddress();

                    geometryKhr.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
                    geometryKhr.geometry.instances = instancesDataKhr;

                    storeTemporaryBuffer(instanceDataBufferH);

                } else if constexpr (std::is_same_v<T, AccelerationStructureGeometryAabbsData>) {
                    VkAccelerationStructureGeometryAabbsDataKHR aabbsDataKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR };
                    aabbsDataKhr.stride = arg.stride;

                    if (arg.data.isValid()) {
                        VulkanBuffer *buffer = vulkanResourceManager->getBuffer(arg.data);
                        aabbsDataKhr.data.deviceAddress = buffer->bufferDeviceAddress() + arg.dataOffset;
                    }

                    geometryKhr.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
                    geometryKhr.geometry.aabbs = aabbsDataKhr;
                } else {
                    static_assert(always_false<T>, "non-exhaustive visitor!");
                }
            },
                       geometry);

            geometries.push_back(geometryKhr);
        }

        VkAccelerationStructureBuildGeometryInfoKHR geometryInfoKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
        geometryInfoKhr.mode = accelerationStructureModeToVkStructureMode(geometryBuildInfo.mode);
        geometryInfoKhr.pGeometries = geometries.data();
        geometryInfoKhr.geometryCount = geometries.size();

        // Source Structure to use when doing updates
        const bool isUpdateMode = geometryInfoKhr.mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
        if (geometryBuildInfo.sourceStructure.isValid() && isUpdateMode) {
            VulkanAccelerationStructure *srcAccelerationStructure = vulkanResourceManager->getAccelerationStructure(geometryBuildInfo.destinationStructure);
            geometryInfoKhr.srcAccelerationStructure = srcAccelerationStructure->accelerationStructure;
        }

        if (geometryBuildInfo.destinationStructure.isValid()) {
            VulkanAccelerationStructure *dstAccelerationStructure = vulkanResourceManager->getAccelerationStructure(geometryBuildInfo.destinationStructure);

            // Create temporary ScratchBuffer (size is not the same whether we are updating or building)
            const VkDeviceSize scratchBufferSize = isUpdateMode ? dstAccelerationStructure->buildSizes.updateScratchSize : dstAccelerationStructure->buildSizes.buildScratchSize;
            Handle<Buffer_t> scratchBufferH = VulkanAccelerationStructure::createAccelerationBuffer(deviceHandle, vulkanResourceManager, scratchBufferSize);
            VulkanBuffer *scratchBuffer = vulkanResourceManager->getBuffer(scratchBufferH);

            geometryInfoKhr.type = accelerationStructureTypeToVkAccelerationStructureType(dstAccelerationStructure->type);
            geometryInfoKhr.scratchData.deviceAddress = scratchBuffer->bufferDeviceAddress();
            geometryInfoKhr.dstAccelerationStructure = dstAccelerationStructure->accelerationStructure;
            geometryInfoKhr.flags = dstAccelerationStructure->buildFlags;

            storeTemporaryBuffer(scratchBufferH);
        }

        infos.push_back(geometryInfoKhr);

        // Build Range Infos for the geometries
        assert(geometries.size() == geometryBuildInfo.buildRangeInfos.size());

        std::vector<VkAccelerationStructureBuildRangeInfoKHR> innerRanges;
        for (const auto &buildRangeInfo : geometryBuildInfo.buildRangeInfos) {
            {
                VkAccelerationStructureBuildRangeInfoKHR rangeInfoKhr;
                rangeInfoKhr.primitiveCount = buildRangeInfo.primitiveCount;
                rangeInfoKhr.primitiveOffset = buildRangeInfo.primitiveOffset;
                rangeInfoKhr.firstVertex = buildRangeInfo.firstVertex;
                rangeInfoKhr.transformOffset = buildRangeInfo.transformOffset;

                innerRanges.push_back(rangeInfoKhr);
            }
        }
        ranges.push_back(innerRanges);
    }

    std::vector<VkAccelerationStructureBuildRangeInfoKHR *> rangePtrs;
    for (auto &range : ranges) {
        rangePtrs.push_back(range.data());
    }

    if (vulkanDevice->vkCmdBuildAccelerationStructuresKHR != nullptr) {
        vulkanDevice->vkCmdBuildAccelerationStructuresKHR(commandBuffer,
                                                          options.buildGeometryInfos.size(),
                                                          infos.data(),
                                                          rangePtrs.data());
    }
}

void VulkanCommandRecorder::beginDebugLabel(const DebugLabelOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkCmdBeginDebugUtilsLabelEXT != nullptr) {
        VkDebugUtilsLabelEXT labelsInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = options.label.data(),
        };
        std::memcpy(labelsInfo.color, options.color, 4 * sizeof(float));
        vulkanDevice->vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &labelsInfo);
    }
}

void VulkanCommandRecorder::endDebugLabel()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkCmdBeginDebugUtilsLabelEXT != nullptr)
        vulkanDevice->vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

Handle<CommandBuffer_t> VulkanCommandRecorder::finish()
{
    VulkanCommandBuffer *commandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer->finish();
    return commandBufferHandle;
}

} // namespace KDGpu
