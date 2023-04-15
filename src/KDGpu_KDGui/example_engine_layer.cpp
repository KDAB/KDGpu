#include "example_engine_layer.h"

#include <KDGpu_KDGui/engine.h>
#include <KDGpu_KDGui/imgui_input_handler.h>
#include <KDGpu_KDGui/imgui_item.h>
#include <KDGpu_KDGui/imgui_renderer.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture_options.h>

#include <KDGui/gui_application.h>

#include <algorithm>

namespace KDGpuKDGui {

ExampleEngineLayer::ExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
}

ExampleEngineLayer::ExampleEngineLayer(const SampleCountFlagBits samples)
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
    , m_samples(samples)
{
}

ExampleEngineLayer::~ExampleEngineLayer()
{
}

void ExampleEngineLayer::recreateSwapChain()
{
    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = m_surface,
        .format = m_swapchainFormat,
        .imageExtent = { .width = m_window->width(), .height = m_window->height() },
        .presentMode = m_presentMode,
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
        .format = m_depthFormat,
        .extent = { m_window->width(), m_window->height(), 1 },
        .mipLevels = 1,
        .samples = m_samples,
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();
}

void ExampleEngineLayer::waitForUploadBufferData(const Handle<Buffer_t> &destinationBuffer,
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

void ExampleEngineLayer::uploadBufferData(const Handle<Buffer_t> &destinationBuffer,
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

void ExampleEngineLayer::waitForUploadTextureData(const Handle<Texture_t> &destinationTexture,
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
    m_queue.submit({ .commandBuffers = { commandBuffer } });

    // Block until the transfer is done
    m_queue.waitUntilIdle();
}

void ExampleEngineLayer::uploadTextureData(const Handle<Texture_t> &destinationTexture,
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
    auto fence = m_device.createFence({ .createSignalled = false });
    auto fenceHandle = fence.handle();
    m_stagingBuffers.emplace_back(StagingBuffer{
            .fence = std::move(fence),
            .buffer = std::move(stagingBuffer),
            .commandBuffer = std::move(commandBuffer) });

    m_queue.submit({ .commandBuffers = { commandBufferHandle }, .signalFence = fenceHandle });
}

void ExampleEngineLayer::releaseStagingBuffers()
{
    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const StagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount)
        SPDLOG_INFO("Released {} staging buffers", removedCount);
}

void ExampleEngineLayer::onAttached()
{
    m_window = std::make_unique<View>();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->objectName(),
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);

    // Create a drawable surface
    m_surface = m_window->createSurface(m_instance);

    // Create a device and a queue to use
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_device = std::move(defaultDevice.device);
    m_queue = m_device.queues()[0];

    // Choose a presentation mode from the ones supported
    constexpr std::array<PresentMode, 4> preferredPresentModes = {
        PresentMode::Mailbox,
        PresentMode::FifoRelaxed,
        PresentMode::Fifo,
        PresentMode::Immediate
    };
    const auto &availableModes = defaultDevice.adapter->swapchainProperties(m_surface).presentModes;
    for (const auto &presentMode : preferredPresentModes) {
        const auto it = std::find(availableModes.begin(), availableModes.end(), presentMode);
        if (it != availableModes.end()) {
            m_presentMode = presentMode;
            break;
        }
    }

    // Create the ImGui overlay item
    m_imguiOverlay = std::make_unique<ImGuiItem>(&m_device);

    // TODO: Move swapchain handling to View?
    recreateSwapChain();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_presentCompleteSemaphores[i] = m_device.createGpuSemaphore();
        m_renderCompleteSemaphores[i] = m_device.createGpuSemaphore();
    }
}

void ExampleEngineLayer::onDetached()
{
    cleanupScene();

    m_imguiOverlay = {};

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

} // namespace KDGpuKDGui
