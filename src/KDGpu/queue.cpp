/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "queue.h"

#include <KDGpu/buffer_options.h>
#include <KDGpu/command_recorder.h>
#include <KDGpu/graphics_api.h>
#include <KDGpu/queue.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_queue.h>
#include <KDGpu/api/api_device.h>

namespace KDGpu {

Queue::Queue()
{
}

Queue::Queue(GraphicsApi *api, const Handle<Device_t> &device, const QueueDescription &description)
    : m_api(api)
    , m_device(device)
    , m_queue(description.queue)
    , m_flags(description.flags)
    , m_timestampValidBits(description.timestampValidBits)
    , m_minImageTransferGranularity(description.minImageTransferGranularity)
    , m_queueTypeIndex(description.queueTypeIndex)
{
}

Queue::~Queue()
{
}

void Queue::waitUntilIdle()
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    apiQueue->waitUntilIdle();
}

void Queue::submit(const SubmitOptions &options)
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    apiQueue->submit(options);
}

PresentResult Queue::present(const PresentOptions &options)
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    return apiQueue->present(options);
}

std::vector<PresentResult> Queue::lastPerSwapchainPresentResults() const
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    return apiQueue->lastPerSwapchainPresentResults();
}

void Queue::waitForUploadBufferData(const WaitForBufferUploadOptions &options)
{
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = options.byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    Buffer stagingBuffer(m_api, m_device, bufferOptions, options.data);

    const CommandRecorderOptions commandRecorderOptions = {
        .queue = m_queue
    };
    CommandRecorder commandRecorder(m_api, m_device, commandRecorderOptions);
    const BufferCopy copyCmd = {
        .src = stagingBuffer,
        .srcOffset = 0,
        .dst = options.destinationBuffer,
        .dstOffset = options.dstOffset,
        .byteSize = options.byteSize
    };
    commandRecorder.copyBuffer(copyCmd);
    auto commandBuffer = commandRecorder.finish();

    submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    waitUntilIdle();
}

UploadStagingBuffer Queue::uploadBufferData(const BufferUploadOptions &options)
{
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = options.byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    Buffer stagingBuffer(m_api, m_device, bufferOptions, options.data);

    const CommandRecorderOptions commandRecorderOptions = {
        .queue = m_queue
    };
    CommandRecorder commandRecorder(m_api, m_device, commandRecorderOptions);
    const BufferCopy copyCmd = {
        .src = stagingBuffer,
        .srcOffset = 0,
        .dst = options.destinationBuffer,
        .dstOffset = options.dstOffset,
        .byteSize = options.byteSize
    };
    commandRecorder.copyBuffer(copyCmd);

    // Insert a buffer barrier
    const BufferMemoryBarrierOptions bufferBarrierOptions = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .srcMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .dstStages = options.dstStages,
        .dstMask = options.dstMask,
        .buffer = options.destinationBuffer
    };
    commandRecorder.bufferMemoryBarrier(bufferBarrierOptions);

    auto commandBuffer = commandRecorder.finish();
    auto commandBufferHandle = commandBuffer.handle();

    // We will use a fence to know when it is safe to destroy the staging buffer
    Fence fence(m_api, m_device, FenceOptions{ .createSignalled = false });
    auto fenceHandle = fence.handle();
    UploadStagingBuffer uploadStagingBuffer{
        .fence = std::move(fence),
        .buffer = std::move(stagingBuffer),
        .commandBuffer = std::move(commandBuffer)
    };

    submit({ .commandBuffers = { commandBufferHandle },
             .signalFence = fenceHandle });

    return uploadStagingBuffer;
}

void Queue::waitForUploadTextureData(const WaitForTextureUploadOptions &options)
{
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = options.byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    Buffer stagingBuffer{ m_api, m_device, bufferOptions, options.data };

    const CommandRecorderOptions commandRecorderOptions = {
        .queue = m_queue
    };
    CommandRecorder commandRecorder(m_api, m_device, commandRecorderOptions);

    // Specify which subresource we will be copying and transitioning
    const TextureSubresourceRange range = {
        .aspectMask = TextureAspectFlags(TextureAspectFlagBits::ColorBit),
        .levelCount = 1
    };

    // We first need to transition the texture into the TextureLayout::TransferDstOptimal layout
    const TextureMemoryBarrierOptions toTransferDstOptimal = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TopOfPipeBit),
        .dstStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .dstMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .oldLayout = options.oldLayout,
        .newLayout = TextureLayout::TransferDstOptimal,
        .texture = options.destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toTransferDstOptimal);

    // Now we perform the staging buffer to texture copy operation
    // clang-format off
    const BufferToTextureCopy copyCmd = {
        .srcBuffer = stagingBuffer,
        .dstTexture = options.destinationTexture,
        .dstTextureLayout = TextureLayout::TransferDstOptimal,
        .regions = options.regions
    };
    // clang-format on
    commandRecorder.copyBufferToTexture(copyCmd);

    // Finally, we transition the texture to the specified final layout
    const TextureMemoryBarrierOptions toFinalLayout = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .srcMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .dstStages = PipelineStageFlags(PipelineStageFlagBit::AllGraphicsBit), // Could be used anywhere in pipeline
        .dstMask = AccessFlags(AccessFlagBit::MemoryReadBit),
        .oldLayout = TextureLayout::TransferDstOptimal,
        .newLayout = options.newLayout,
        .texture = options.destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toFinalLayout);

    auto commandBuffer = commandRecorder.finish();
    submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    waitUntilIdle();
}

UploadStagingBuffer Queue::uploadTextureData(const TextureUploadOptions &options)
{
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = options.byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    Buffer stagingBuffer{ m_api, m_device, bufferOptions, options.data };

    const CommandRecorderOptions commandRecorderOptions = {
        .queue = m_queue
    };
    CommandRecorder commandRecorder(m_api, m_device, commandRecorderOptions);

    // Specify which subresource we will be copying and transitioning
    const TextureSubresourceRange range = {
        .aspectMask = TextureAspectFlags(TextureAspectFlagBits::ColorBit),
        .levelCount = 1
    };

    // We first need to transition the texture into the TextureLayout::TransferDstOptimal layout
    const TextureMemoryBarrierOptions toTransferDstOptimal = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TopOfPipeBit),
        .dstStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .dstMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .oldLayout = options.oldLayout,
        .newLayout = TextureLayout::TransferDstOptimal,
        .texture = options.destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toTransferDstOptimal);

    // Now we perform the staging buffer to texture copy operation
    // clang-format off
    const BufferToTextureCopy copyCmd = {
        .srcBuffer = stagingBuffer,
        .dstTexture = options.destinationTexture,
        .dstTextureLayout = TextureLayout::TransferDstOptimal,
        .regions = options.regions
    };
    // clang-format on
    commandRecorder.copyBufferToTexture(copyCmd);

    // Finally, we transition the texture to the specified final layout
    const TextureMemoryBarrierOptions toFinalLayout = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .srcMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .dstStages = options.dstStages,
        .dstMask = options.dstMask,
        .oldLayout = TextureLayout::TransferDstOptimal,
        .newLayout = options.newLayout,
        .texture = options.destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toFinalLayout);

    auto commandBuffer = commandRecorder.finish();
    auto commandBufferHandle = commandBuffer.handle();

    // We will use a fence to know when it is safe to destroy the staging buffer
    Fence fence(m_api, m_device, FenceOptions{ .createSignalled = false });
    auto fenceHandle = fence.handle();
    UploadStagingBuffer uploadStagingBuffer{
        .fence = std::move(fence),
        .buffer = std::move(stagingBuffer),
        .commandBuffer = std::move(commandBuffer)
    };

    submit({ .commandBuffers = { commandBufferHandle }, .signalFence = fenceHandle });

    return uploadStagingBuffer;
}

} // namespace KDGpu
