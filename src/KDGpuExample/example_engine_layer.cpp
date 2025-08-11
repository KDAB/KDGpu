/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "example_engine_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/imgui_input_handler.h>
#include <KDGpuExample/imgui_item.h>
#include <KDGpuExample/imgui_renderer.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture_options.h>

#include <KDGui/gui_application.h>

#include <KDUtils/logging.h>

#include <imgui.h>

#include <algorithm>

using namespace KDGpu;

namespace KDGpuExample {

ExampleEngineLayer::ExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
    m_logger = KDUtils::Logger::logger("engine-layer", spdlog::level::info);
    m_samples.valueChanged().connect([this]() { recreateSampleDependentResources(); }).release();
}

ExampleEngineLayer::~ExampleEngineLayer()
{
}

void ExampleEngineLayer::recreateSwapChain()
{
    const AdapterSwapchainProperties swapchainProperties = m_device.adapter()->swapchainProperties(m_surface);
    const SurfaceCapabilities &surfaceCapabilities = swapchainProperties.capabilities;

    m_swapchainExtent = {
        .width = std::clamp(m_window->width(), surfaceCapabilities.minImageExtent.width,
                            surfaceCapabilities.maxImageExtent.width),
        .height = std::clamp(m_window->height(), surfaceCapabilities.minImageExtent.height,
                             surfaceCapabilities.maxImageExtent.height),
    };

    // Create a swapchain of images that we will render to.
    const SwapchainOptions swapchainOptions = {
        .surface = m_surface,
        .format = m_swapchainFormat,
        .minImageCount = getSuitableImageCount(surfaceCapabilities),
        .imageExtent = m_swapchainExtent,
        .imageUsageFlags = m_swapchainUsageFlags,
        .compositeAlpha = m_compositeAlpha,
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

    recreateDepthTexture();

    m_capabilitiesString = surfaceCapabilitiesToString(m_device.adapter()->swapchainProperties(m_surface).capabilities);
}

void ExampleEngineLayer::recreateDepthTexture()
{
    // Create a depth texture to use for depth-correct rendering
    TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_depthFormat,
        .extent = { m_swapchainExtent.width, m_swapchainExtent.height, 1 },
        .mipLevels = 1,
        .samples = m_samples.get(),
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit | m_depthTextureUsageFlags,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();
}

void ExampleEngineLayer::recreateSampleDependentResources()
{
    // create the resources only if they exist already
    if (m_imguiOverlay != nullptr) {
        recreateImGuiOverlay();
    }
    if (m_depthTexture.isValid()) {
        recreateDepthTexture();
    }
}

void ExampleEngineLayer::uploadBufferData(const BufferUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadBufferData(options));
}

void ExampleEngineLayer::uploadTextureData(const TextureUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadTextureData(options));
}

void ExampleEngineLayer::releaseStagingBuffers()
{
    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const UploadStagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount) {
        SPDLOG_LOGGER_INFO(m_logger, "Released {} staging buffers", removedCount);
    }
}

void ExampleEngineLayer::recreateImGuiOverlay()
{
    m_imguiOverlay = std::make_unique<ImGuiItem>(&m_device, &m_queue);
    m_window->scaleFactor.valueChanged().connect([this](const float scale_factor) {
                                            m_imguiOverlay->updateScale(scale_factor);
                                        })
            .release();

    m_imguiOverlay->initialize(m_window->scaleFactor.get(), m_samples.get(), m_swapchainFormat, m_depthFormat);
}

void ExampleEngineLayer::drawImGuiOverlay(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin(
            "Basic Info",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::Text("App: %s", KDGui::GuiApplication::instance()->applicationName().data());
    ImGui::Text("GPU: %s", m_device.adapter()->properties().deviceName.c_str());
    const auto fps = engine()->fps();
    ImGui::Text("%.2f ms/frame (%.1f fps)", (1000.0f / fps), fps);

    if (ImGui::Button("Surface Capabilities"))
        m_showSurfaceCapabilities = !m_showSurfaceCapabilities;
    ImGui::End();

    if (m_showSurfaceCapabilities) {
        ImGui::Begin("Capabilities:", &m_showSurfaceCapabilities);
        ImGui::Text("%s", m_capabilitiesString.data());
        ImGui::End();
    }

    for (const auto &func : m_imGuiOverlayDrawFunctions)
        func(ctx);
}

void ExampleEngineLayer::renderImGuiOverlay(RenderPassCommandRecorder *recorder,
                                            uint32_t inFlightIndex,
                                            RenderPass *currentRenderPass,
                                            int lastSubpassIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    const Extent2D extent{ m_window->width(), m_window->height() };
    m_imguiOverlay->render(recorder, extent, inFlightIndex, currentRenderPass, lastSubpassIndex);
}

void ExampleEngineLayer::renderImGuiOverlayDynamic(KDGpu::RenderPassCommandRecorder *recorder,
                                                   uint32_t inFlightIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    const Extent2D extent{ m_window->width(), m_window->height() };
    m_imguiOverlay->renderDynamic(recorder, extent, inFlightIndex);
}

void ExampleEngineLayer::registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func)
{
    m_imGuiOverlayDrawFunctions.push_back(func);
}

void ExampleEngineLayer::clearImGuiOverlayDrawFunctions()
{
    m_imGuiOverlayDrawFunctions.clear();
}

void ExampleEngineLayer::onAttached()
{
    m_window = std::make_unique<KDGpuKDGui::View>();
    m_window->title = KDGui::GuiApplication::instance()->applicationName();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->applicationName(),
        .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);

    // Create a drawable surface
    m_surface = m_window->createSurface(m_instance);

    // Create a device and a queue to use
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_adapter = defaultDevice.adapter;
    m_device = std::move(defaultDevice.device);
    m_queue = m_device.queues()[0];

    const AdapterSwapchainProperties swapchainProperties = m_device.adapter()->swapchainProperties(m_surface);

    // Choose a presentation mode from the ones supported
    constexpr std::array<PresentMode, 4> preferredPresentModes = {
        PresentMode::Mailbox,
        PresentMode::FifoRelaxed,
        PresentMode::Fifo,
        PresentMode::Immediate
    };
    const auto &availableModes = swapchainProperties.presentModes;
    for (const auto &presentMode : preferredPresentModes) {
        const auto it = std::find(availableModes.begin(), availableModes.end(), presentMode);
        if (it != availableModes.end()) {
            m_presentMode = presentMode;
            break;
        }
    }

    // Try to ensure that the chosen format is supported
    m_swapchainFormat =
            [this, &swapchainProperties]() {
                for (const auto &availableFormat : swapchainProperties.formats) {
                    if (availableFormat.format == m_swapchainFormat &&
                        availableFormat.colorSpace == ColorSpace::SRgbNonlinear) {
                        return availableFormat.format;
                    }
                }
                // Fallback to first listed available format
                return swapchainProperties.formats[0].format;
            }();

    // Choose a depth format from the ones supported
    constexpr std::array<Format, 5> preferredDepthFormat = {
        Format::D24_UNORM_S8_UINT,
        Format::D16_UNORM_S8_UINT,
        Format::D32_SFLOAT_S8_UINT,
        Format::D16_UNORM,
        Format::D32_SFLOAT,
    };
    for (const auto &depthFormat : preferredDepthFormat) {
        const FormatProperties formatProperties = defaultDevice.adapter->formatProperties(depthFormat);
        if (formatProperties.optimalTilingFeatures & FormatFeatureFlagBit::DepthStencilAttachmentBit) {
            m_depthFormat = depthFormat;
            break;
        }
    }

    // Try to ensure that the chosen alpha composite mode is supported
    m_compositeAlpha =
            [this, &swapchainProperties]() {
                const auto supportedCompositeAlpha = swapchainProperties.capabilities.supportedCompositeAlpha;

                if (supportedCompositeAlpha.testFlag(m_compositeAlpha))
                    return m_compositeAlpha;

                // Try to return a single, known, supported alpha bit
                constexpr std::array<CompositeAlphaFlagBits, 4> compositeAlphaBits = {
                    CompositeAlphaFlagBits::OpaqueBit,
                    CompositeAlphaFlagBits::PreMultipliedBit,
                    CompositeAlphaFlagBits::PostMultipliedBit,
                    CompositeAlphaFlagBits::InheritBit
                };
                for (const auto alphaBit : compositeAlphaBits) {
                    if (supportedCompositeAlpha.testFlag(alphaBit))
                        return alphaBit;
                }

                // If all else fails, do not change
                return m_compositeAlpha;
            }();

    // TODO: Move swapchain handling to View?
    recreateSwapChain();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_presentCompleteSemaphores[i] = m_device.createGpuSemaphore();
    }

    // https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html
    // We index renderCompleteSemaphore by swapchain image index rather than
    // frame in flight index to ensure presentation has been fully completed
    // before we try to reuse a swapchain image in subsequent frames (waiting
    // on the fence we use during submission doesn't guarantee present operations
    // have been completed).
    m_renderCompleteSemaphores.reserve(m_swapchainViews.size());
    for (size_t i = 0, m = m_swapchainViews.size(); i < m; ++i) {
        m_renderCompleteSemaphores.emplace_back(std::move(m_device.createGpuSemaphore()));
    }

    constexpr std::array availableSampleCounts{
        SampleCountFlagBits::Samples1Bit,
        SampleCountFlagBits::Samples2Bit,
        SampleCountFlagBits::Samples4Bit,
        SampleCountFlagBits::Samples8Bit,
        SampleCountFlagBits::Samples16Bit,
        SampleCountFlagBits::Samples32Bit,
        SampleCountFlagBits::Samples64Bit,
    };

    // get all of the supported sample counts for the hardware
    {
        auto supported = m_device.adapter()->properties().limits.framebufferColorSampleCounts.toInt();
        assert(supported);

        for (auto sample : availableSampleCounts) {
            if (static_cast<int>(sample) & supported)
                m_supportedSampleCounts.push_back(sample);
        }
    }

    recreateImGuiOverlay();

    initializeScene();
}

void ExampleEngineLayer::onDetached()
{
    m_imguiOverlay->cleanup();
    cleanupScene();

    m_imguiOverlay = {};

    m_presentCompleteSemaphores = {};
    m_renderCompleteSemaphores.clear();
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

    // Call our imgui drawing function
    ImGui::NewFrame();
    drawImGuiOverlay(context);

    // TODO: Draw any additional registered imgui drawing callbacks

    // Process the ImGui drawing functions to generate geometry and commands. The actual buffers will be updated
    // and commands translated by the ImGuiRenderer later in the frame.
    ImGui::Render();
}

void ExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
    // Forward window events to the ImGui overlay
    if (target == m_window.get())
        m_imguiOverlay->event(target, ev);
}

} // namespace KDGpuExample
