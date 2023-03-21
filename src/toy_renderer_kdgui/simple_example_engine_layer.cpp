#include "simple_example_engine_layer.h"

#include <toy_renderer_kdgui/engine.h>

#include <toy_renderer/buffer_options.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/texture_options.h>

SimpleExampleEngineLayer::SimpleExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
}

SimpleExampleEngineLayer::~SimpleExampleEngineLayer()
{
}

void SimpleExampleEngineLayer::recreateSwapChain()
{
    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = m_surface,
        .format = m_swapchainFormat,
        .imageExtent = { .width = m_window->width(), .height = m_window->height() },
        .oldSwapchain = m_swapchain,
    };

    // Create swapchain and destroy previous one implicitly
    m_swapchain = m_device.createSwapchain(swapchainOptions);

    const auto &swapchainTextures = m_swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();

    m_swapchainViews.clear();
    m_swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        m_swapchainViews.push_back(std::move(view));
    }

    // Create a depth texture to use for depth-correct rendering
    TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = Format::D24_UNORM_S8_UINT,
        .extent = { m_window->width(), m_window->height(), 1 },
        .mipLevels = 1,
        .usage = TextureUsageFlags(TextureUsageFlagBits::DepthStencilAttachmentBit),
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();
}

// TODO: Implement a non-blocking buffer upload by way of a memory barrier
// TODO: Later implement a non-blocking buffer upload via staging buffer on transfer queue and transfer of ownership.
void SimpleExampleEngineLayer::waitForUploadBufferData(const Handle<Buffer_t> &destinationBuffer,
                                                       const void *data,
                                                       DeviceSize byteSize,
                                                       DeviceSize dstOffset)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlags(BufferUsageFlagBits::TransferSrcBit),
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = m_device.createBuffer(bufferOptions, data);

    auto commandRecorder = m_device.createCommandRecorder();
    const BufferCopy copyCmd = {
        .src = stagingBuffer,
        .srcOffset = 0,
        .dst = destinationBuffer,
        .dstOffset = dstOffset,
        .byteSize = byteSize
    };
    commandRecorder.copyBuffer(copyCmd);
    auto commandBuffer = commandRecorder.finish();

    m_queue.submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    m_queue.waitUntilIdle();
}

void SimpleExampleEngineLayer::uploadBufferData(const Handle<Buffer_t> &destinationBuffer,
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
        .usage = BufferUsageFlags(BufferUsageFlagBits::TransferSrcBit),
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = m_device.createBuffer(bufferOptions, data);

    auto commandRecorder = m_device.createCommandRecorder();
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
    auto fence = m_device.createFence({ .createSignalled = false });
    auto fenceHandle = fence.handle();
    m_stagingBuffers.emplace_back(StagingBuffer{
            .fence = std::move(fence),
            .buffer = std::move(stagingBuffer),
            .commandBuffer = std::move(commandBuffer) });

    m_queue.submit({ .commandBuffers = { commandBufferHandle },
                     .signalFence = fenceHandle });
}

void SimpleExampleEngineLayer::waitForUploadTextureData(const Handle<Texture_t> &destinationTexture,
                                                        const void *data,
                                                        DeviceSize byteSize,
                                                        Extent3D textureExtent,
                                                        Offset3D textureOffset,
                                                        TextureLayout oldLayout,
                                                        TextureLayout newLayout)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlags(BufferUsageFlagBits::TransferSrcBit),
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = m_device.createBuffer(bufferOptions, data);

    auto commandRecorder = m_device.createCommandRecorder();

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
        .regions = {{
            .imageSubResource = { .aspectMask = TextureAspectFlags(TextureAspectFlagBits::ColorBit) },
            .imageOffset = {
                .x = textureOffset.x,
                .y = textureOffset.y,
                .z = textureOffset.z },
            .imageExtent = {
                .width = textureExtent.width,
                .height = textureExtent.height,
                .depth = textureExtent.depth
            }
        }},
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
    m_queue.submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    m_queue.waitUntilIdle();
}

void SimpleExampleEngineLayer::uploadTextureData(const Handle<Texture_t> &destinationTexture,
                                                 PipelineStageFlags dstStages,
                                                 AccessFlags dstMask,
                                                 const void *data,
                                                 DeviceSize byteSize,
                                                 Extent3D textureExtent,
                                                 Offset3D textureOffset,
                                                 TextureLayout oldLayout,
                                                 TextureLayout newLayout)
{
    // Buffer upload via a staging buffer using our main queue
    // Create a staging buffer and upload initial data to it by map(), memcpy(), unmap().
    BufferOptions bufferOptions = {
        .size = byteSize,
        .usage = BufferUsageFlags(BufferUsageFlagBits::TransferSrcBit),
        .memoryUsage = MemoryUsage::CpuToGpu // So we can map it to CPU address space
    };
    auto stagingBuffer = m_device.createBuffer(bufferOptions, data);

    auto commandRecorder = m_device.createCommandRecorder();

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
        .regions = {{
            .imageSubResource = { .aspectMask = TextureAspectFlags(TextureAspectFlagBits::ColorBit) },
            .imageOffset = {
                .x = textureOffset.x,
                .y = textureOffset.y,
                .z = textureOffset.z },
            .imageExtent = {
                .width = textureExtent.width,
                .height = textureExtent.height,
                .depth = textureExtent.depth
            }
        }},
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
    auto fence = m_device.createFence({ .createSignalled = false });
    auto fenceHandle = fence.handle();
    m_stagingBuffers.emplace_back(StagingBuffer{
            .fence = std::move(fence),
            .buffer = std::move(stagingBuffer),
            .commandBuffer = std::move(commandBuffer) });

    m_queue.submit({ .commandBuffers = { commandBufferHandle }, .signalFence = fenceHandle });
}

void SimpleExampleEngineLayer::releaseStagingBuffers()
{
    // TODO: Recycle fences and use regions of a large persistent staging buffer

    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const StagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount)
        SPDLOG_INFO("Released {} staging buffers", removedCount);
}

void SimpleExampleEngineLayer::onAttached()
{
    m_window = std::make_unique<View>();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    // TODO: Pass these options in to the layer
    InstanceOptions instanceOptions = {
        .applicationName = "03_hello_triangle_simple",
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);

    // Create a drawable surface
    m_surface = m_window->createSurface(m_instance);

    // Create a device and a queue to use
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_device = std::move(defaultDevice.device);
    m_queue = m_device.queues()[0];

    // TODO: Move swapchain handling to View?
    recreateSwapChain();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_presentCompleteSemaphores[i] = m_device.createGpuSemaphore();
        m_renderCompleteSemaphores[i] = m_device.createGpuSemaphore();
    }

    initializeScene();
}

void SimpleExampleEngineLayer::onDetached()
{
    cleanupScene();

    m_presentCompleteSemaphores = {};
    m_renderCompleteSemaphores = {};
    m_depthTextureView = {};
    m_depthTexture = {};
    m_swapchainViews.clear();
    m_swapchain = {};
    m_queue = {};
    m_device = {};
    m_surface = {};
    m_instance = {};
    m_window = {};
}

void SimpleExampleEngineLayer::update()
{
    // Release any staging buffers we are done with
    releaseStagingBuffers();

    // Call updateScene() function to update scene state.
    updateScene();

    // Obtain swapchain image view
    m_inFlightIndex = engine()->frameNumber() % MAX_FRAMES_IN_FLIGHT;
    const auto result = m_swapchain.getNextImageIndex(m_currentSwapchainImageIndex,
                                                      m_presentCompleteSemaphores[m_inFlightIndex]);
    if (result == AcquireImageResult::OutOfDate) {
        // We need to recreate swapchain
        recreateSwapChain();
        // Handle any changes that would be needed when a swapchain resize occurs
        resize();
        // Early return as we need to retry to retrieve the image index
        return;
    } else if (result != AcquireImageResult::Success) {
        // Something went wrong and we can't recover from it
        return;
    }

    // Call subclass render() function to record and submit drawing commands
    render();

    // Present the swapchain image
    // clang-format off
    PresentOptions presentOptions = {
        .waitSemaphores = { m_renderCompleteSemaphores[m_inFlightIndex] },
        .swapchainInfos = {{
            .swapchain = m_swapchain,
            .imageIndex = m_currentSwapchainImageIndex
        }}
    };
    // clang-format on
    m_queue.present(presentOptions);

    // TODO: Use fences to not block here
    m_device.waitUntilIdle();
}
