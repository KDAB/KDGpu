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
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_queue.h>
#include <KDGpu/api/api_device.h>

#include <numeric>
#include <algorithm>

namespace KDGpu {

/**
    @class SubmitOptions
    @brief Holds information required to perform a queue submission.

    @var commandBuffers holds a vector of handles to the CommandBuffer instances that needs to be submitted for execution.
    @var waitSemaphores holds a vector of handles to GpuSemaphore instances commands will have to wait for before execution begin
    @var signalSemaphores holds a vector of handles to GpuSemaphore instances that will be signalled when execution of the commands completes
    @var signalFence holds a handle to Fence instance to be signalled when execution of the commands completes

    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>
    @sa KDGpu::GpuSemaphore
    @sa KDGpu::Fence
    @sa KDGpu::CommandBuffer
*/

/**
    @class Queue
    @brief Queue is used to submit commands for execution and optionally present content
    @ingroup public
    @headerfile queue.h <KDGpu/queue.h>

    @code{.cpp}
    using namespace KDGpu;

    Adapter *selectedAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = selectedAdapter->createDevice();
    Queue queue = device.queues()[0];

    CommandRecorder commandRecorder = device.createCommandRecorder();
    ...
    const CommandBuffer commands = commandRecorder.finish();

    queue.submit(SubmitOptions{
                .commandBuffers = { commands },
    });

    @endcode

    @sa Device::queues
 */

/**
    @fn Queue::handle()
    @brief Returns the handle used to retrieve the underlying API specific Queue
    @sa ResourceManager
 */

/**
    @fn Queue::isValid()
    @brief Convenience function to check whether the Queue is actually referencing a valid API specific resource
 */
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

/**
 * @brief Forces a CPU side blocking wait until all pending commands on the queue have completed their execution.
 */
void Queue::waitUntilIdle()
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    apiQueue->waitUntilIdle();
}

/**
 * @brief Submit commands for execution based on the SubmitOptions @a options provided
 */
void Queue::submit(const SubmitOptions &options)
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    apiQueue->submit(options);
}

/**
 * @brief Request the Queue present content to the swapchains referenced in the PresentOptions @a options
 */
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
        .memoryUsage = MemoryUsage::CpuOnly // Use a CPU heap for the staging buffer
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
        .memoryUsage = MemoryUsage::CpuOnly // Use a CPU heap for the staging buffer
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

namespace {

TextureSubresourceRange createRangeFromRegions(const std::vector<BufferTextureCopyRegion> regions)
{
    const auto maxMipLayerPair = std::accumulate(
            regions.begin(), regions.end(), std::make_pair(0U, 0U),
            [](std::pair<uint32_t, uint32_t> acc, const BufferTextureCopyRegion &region) {
                return std::make_pair(std::max(acc.first, region.textureSubResource.mipLevel),
                                      std::max(acc.second, region.textureSubResource.baseArrayLayer));
            });

    return TextureSubresourceRange{
        .aspectMask = TextureAspectFlags(regions.empty() ? TextureAspectFlagBits::ColorBit : regions.front().textureSubResource.aspectMask),
        .levelCount = maxMipLayerPair.first + 1,
        .layerCount = maxMipLayerPair.second + 1
    };
};

} // namespace

void Queue::waitForUploadTextureData(const WaitForTextureUploadOptions &options)
{
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = options.byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuOnly // Use a CPU heap for the staging buffer
    };
    Buffer stagingBuffer{ m_api, m_device, bufferOptions, options.data };

    const CommandRecorderOptions commandRecorderOptions = {
        .queue = m_queue
    };
    CommandRecorder commandRecorder(m_api, m_device, commandRecorderOptions);

    // Find a suitable subresource we will be copying and transitioning
    const TextureSubresourceRange range = options.range.aspectMask != TextureAspectFlagBits::None ? createRangeFromRegions(options.regions) : options.range;

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
        .dstMask = AccessFlags(AccessFlagBit::InputAttachmentReadBit | AccessFlagBit::ShaderReadBit),
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
        .memoryUsage = MemoryUsage::CpuOnly // Use a CPU heap for the staging buffer
    };
    Buffer stagingBuffer{ m_api, m_device, bufferOptions, options.data };

    const CommandRecorderOptions commandRecorderOptions = {
        .queue = m_queue
    };
    CommandRecorder commandRecorder(m_api, m_device, commandRecorderOptions);

    // Find a suitable subresource we will be copying and transitioning
    const TextureSubresourceRange range = options.range.aspectMask != TextureAspectFlagBits::None ? createRangeFromRegions(options.regions) : options.range;

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
