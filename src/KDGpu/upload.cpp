#include "upload.h"

#include <KDGpu/buffer_options.h>
#include <KDGpu/device.h>
#include <KDGpu/queue.h>
#include <KDGpu/texture.h>

namespace KDGpu {

void waitForUploadBufferData(Device *device,
                             Queue *queue,
                             const Handle<Buffer_t> &destinationBuffer,
                             const void *data,
                             DeviceSize byteSize,
                             DeviceSize dstOffset)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = device->createBuffer(bufferOptions, data);

    auto commandRecorder = device->createCommandRecorder();
    const BufferCopy copyCmd = {
        .src = stagingBuffer,
        .srcOffset = 0,
        .dst = destinationBuffer,
        .dstOffset = dstOffset,
        .byteSize = byteSize
    };
    commandRecorder.copyBuffer(copyCmd);
    auto commandBuffer = commandRecorder.finish();

    queue->submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    queue->waitUntilIdle();
}

UploadStagingBuffer uploadBufferData(Device *device,
                                     Queue *queue,
                                     const Handle<Buffer_t> &destinationBuffer,
                                     PipelineStageFlags dstStages,
                                     AccessFlags dstMask,
                                     const void *data,
                                     DeviceSize byteSize,
                                     DeviceSize dstOffset)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = device->createBuffer(bufferOptions, data);

    auto commandRecorder = device->createCommandRecorder();
    const BufferCopy copyCmd = {
        .src = stagingBuffer,
        .srcOffset = 0,
        .dst = destinationBuffer,
        .dstOffset = dstOffset,
        .byteSize = byteSize
    };
    commandRecorder.copyBuffer(copyCmd);

    // Insert a buffer barrier
    const BufferMemoryBarrierOptions bufferBarrierOptions = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .srcMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .dstStages = dstStages,
        .dstMask = dstMask,
        .buffer = destinationBuffer
    };
    commandRecorder.bufferMemoryBarrier(bufferBarrierOptions);

    auto commandBuffer = commandRecorder.finish();
    auto commandBufferHandle = commandBuffer.handle();

    // We will use a fence to know when it is safe to destroy the staging buffer
    auto fence = device->createFence({ .createSignalled = false });
    auto fenceHandle = fence.handle();
    UploadStagingBuffer uploadStagingBuffer{
        .fence = std::move(fence),
        .buffer = std::move(stagingBuffer),
        .commandBuffer = std::move(commandBuffer)
    };

    queue->submit({ .commandBuffers = { commandBufferHandle },
                    .signalFence = fenceHandle });

    return uploadStagingBuffer;
}

void waitForUploadTextureData(Device *device,
                              Queue *queue,
                              const Handle<Texture_t> &destinationTexture,
                              const void *data,
                              DeviceSize byteSize,
                              TextureLayout oldLayout,
                              TextureLayout newLayout,
                              const std::vector<BufferImageCopyRegion> &regions)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = device->createBuffer(bufferOptions, data);

    auto commandRecorder = device->createCommandRecorder();

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
        .oldLayout = oldLayout,
        .newLayout = TextureLayout::TransferDstOptimal,
        .texture = destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toTransferDstOptimal);

    // Now we perform the staging buffer to texture copy operation
    // clang-format off
    const BufferToTextureCopy copyCmd = {
        .srcBuffer = stagingBuffer,
        .dstTexture = destinationTexture,
        .dstImageLayout = TextureLayout::TransferDstOptimal,
        .regions = regions
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
        .newLayout = newLayout,
        .texture = destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toFinalLayout);

    auto commandBuffer = commandRecorder.finish();
    queue->submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    queue->waitUntilIdle();
}

UploadStagingBuffer uploadTextureData(Device *device,
                                      Queue *queue,
                                      const Handle<Texture_t> &destinationTexture,
                                      PipelineStageFlags dstStages,
                                      AccessFlags dstMask,
                                      const void *data,
                                      DeviceSize byteSize,
                                      TextureLayout oldLayout,
                                      TextureLayout newLayout,
                                      const std::vector<BufferImageCopyRegion> &regions)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlagBits::TransferSrcBit,
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = device->createBuffer(bufferOptions, data);

    auto commandRecorder = device->createCommandRecorder();

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
        .oldLayout = oldLayout,
        .newLayout = TextureLayout::TransferDstOptimal,
        .texture = destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toTransferDstOptimal);

    // Now we perform the staging buffer to texture copy operation
    // clang-format off
    const BufferToTextureCopy copyCmd = {
        .srcBuffer = stagingBuffer,
        .dstTexture = destinationTexture,
        .dstImageLayout = TextureLayout::TransferDstOptimal,
        .regions = regions
    };
    // clang-format on
    commandRecorder.copyBufferToTexture(copyCmd);

    // Finally, we transition the texture to the specified final layout
    const TextureMemoryBarrierOptions toFinalLayout = {
        .srcStages = PipelineStageFlags(PipelineStageFlagBit::TransferBit),
        .srcMask = AccessFlags(AccessFlagBit::TransferWriteBit),
        .dstStages = dstStages,
        .dstMask = dstMask,
        .oldLayout = TextureLayout::TransferDstOptimal,
        .newLayout = newLayout,
        .texture = destinationTexture,
        .range = range
    };
    commandRecorder.textureMemoryBarrier(toFinalLayout);

    auto commandBuffer = commandRecorder.finish();
    auto commandBufferHandle = commandBuffer.handle();

    // We will use a fence to know when it is safe to destroy the staging buffer
    auto fence = device->createFence({ .createSignalled = false });
    auto fenceHandle = fence.handle();
    UploadStagingBuffer uploadStagingBuffer{
        .fence = std::move(fence),
        .buffer = std::move(stagingBuffer),
        .commandBuffer = std::move(commandBuffer)
    };

    queue->submit({ .commandBuffers = { commandBufferHandle }, .signalFence = fenceHandle });

    return uploadStagingBuffer;
}

} // namespace KDGpu
