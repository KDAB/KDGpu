#include "example_engine_layer.h"

#include <KDGpu_KDGui/engine.h>
#include <KDGpu_KDGui/imgui_input_handler.h>
#include <KDGpu_KDGui/imgui_item.h>
#include <KDGpu_KDGui/imgui_renderer.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture_options.h>

#include <KDGui/gui_application.h>

#include <imgui.h>

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
    WaitForBufferUploadOptions options = {
        .destinationBuffer = destinationBuffer,
        .data = data,
        .byteSize = byteSize,
        .dstOffset = dstOffset
    };
    m_queue.waitForUploadBufferData(options);
}

void ExampleEngineLayer::uploadBufferData(const Handle<Buffer_t> &destinationBuffer,
                                          PipelineStageFlags dstStages,
                                          AccessFlags dstMask,
                                          const void *data,
                                          DeviceSize byteSize,
                                          DeviceSize dstOffset)
{
    BufferUploadOptions options = {
        .destinationBuffer = destinationBuffer,
        .dstStages = dstStages,
        .dstMask = dstMask,
        .data = data,
        .byteSize = byteSize,
        .dstOffset = dstOffset
    };
    m_stagingBuffers.emplace_back(m_queue.uploadBufferData(options));
}

void ExampleEngineLayer::waitForUploadTextureData(const Handle<Texture_t> &destinationTexture,
                                                  const void *data,
                                                  DeviceSize byteSize,
                                                  TextureLayout oldLayout,
                                                  TextureLayout newLayout,
                                                  const std::vector<BufferImageCopyRegion> &regions)
{
    WaitForTextureUploadOptions options = {
        .destinationTexture = destinationTexture,
        .data = data,
        .byteSize = byteSize,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .regions = regions
    };
    m_queue.waitForUploadTextureData(options);
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
    TextureUploadOptions options = {
        .destinationTexture = destinationTexture,
        .dstStages = dstStages,
        .dstMask = dstMask,
        .data = data,
        .byteSize = byteSize,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .regions = regions
    };
    m_stagingBuffers.emplace_back(m_queue.uploadTextureData(options));
}

void ExampleEngineLayer::releaseStagingBuffers()
{
    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const UploadStagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount)
        SPDLOG_INFO("Released {} staging buffers", removedCount);
}

void ExampleEngineLayer::drawImGuiOverlay(ImGuiContext *ctx)
{
    // Do-nothing default
    ImGui::ShowDemoWindow();
}

void ExampleEngineLayer::renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    const Extent2D extent{ m_window->width(), m_window->height() };
    m_imguiOverlay->render(recorder, extent, inFlightIndex);
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

    // TODO: Move swapchain handling to View?
    recreateSwapChain();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_presentCompleteSemaphores[i] = m_device.createGpuSemaphore();
        m_renderCompleteSemaphores[i] = m_device.createGpuSemaphore();
    }

    // Create the ImGui overlay item
    m_imguiOverlay = std::make_unique<ImGuiItem>(&m_device);
    m_imguiOverlay->initialize(m_samples, m_swapchainFormat, m_depthFormat);

    initializeScene();
}

void ExampleEngineLayer::onDetached()
{
    m_imguiOverlay->cleanup();
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

void ExampleEngineLayer::update()
{
    ImGuiContext *context = m_imguiOverlay->context();
    ImGui::SetCurrentContext(context);

    // Set frame time and display size.
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = engine()->deltaTimeSeconds();
    io.DisplaySize = ImVec2(static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));

    // Process the input events for ImGui
    m_imguiOverlay->updateInputState();

    // Call our imgui drawing function
    ImGui::NewFrame();
    drawImGuiOverlay(context);

    // TODO: Draw any additional registered imgui drawing callbacks

    // Process the ImGui drawing functions to generate geometry and commands. The actual buffers will be updated
    // and commands translated by the ImGuiRenderer later in the frame.
    ImGui::Render();
}

} // namespace KDGpuKDGui
