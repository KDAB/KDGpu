/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_example_engine_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/imgui_input_handler.h>
#include <KDGpuExample/imgui_item.h>
#include <KDGpuExample/imgui_renderer.h>
#include <KDXr/utils/formatters.h>
#include <KDGpu/texture_options.h>
#include <KDGui/gui_application.h>

#include <imgui.h>

#include <assert.h>

namespace KDGpuExample {

XrExampleEngineLayer::XrExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
    , m_xrApi(std::make_unique<KDXr::OpenXrApi>())
{
    m_logger = spdlog::get("engine");
    if (!m_logger)
        m_logger = spdlog::stdout_color_mt("engine");
}

XrExampleEngineLayer::~XrExampleEngineLayer()
{
}

void XrExampleEngineLayer::onAttached()
{
    // OpenXR Setup
    // Create the instance and debug message handler
    KDXr::InstanceOptions xrInstanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->applicationName(),
        .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0),
        .layers = {}, // No api layers requested
        .extensions = { XR_EXT_DEBUG_UTILS_EXTENSION_NAME, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME }
    };
    m_kdxrInstance = m_xrApi->createInstance(xrInstanceOptions);
    m_kdxrInstance.instanceLost.connect(&XrExampleEngineLayer::onInstanceLost, this);
    const auto properties = m_kdxrInstance.properties();
    SPDLOG_LOGGER_INFO(m_logger, "XR Runtime: {}", properties.runtimeName);
    SPDLOG_LOGGER_INFO(m_logger, "XR Runtime Version: {}", KDXr::getVersionAsString(properties.runtimeVersion));

    m_kdxrSystem = m_kdxrInstance.system();
    const auto systemProperties = m_kdxrSystem->properties();

    // Pick the first application supported View Configuration Type supported by the hardware.
    m_selectedViewConfiguration = m_kdxrSystem->selectViewConfiguration(m_applicationViewConfigurations);
    if (m_selectedViewConfiguration == KDXr::ViewConfigurationType::MaxEnum) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported ViewConfigurationType.");
        throw std::runtime_error("Failed to find a supported ViewConfigurationType.");
    }

    // We will just use the first environment blend mode supported by the system
    m_selectedEnvironmentBlendMode = m_kdxrSystem->environmentBlendModes(m_selectedViewConfiguration)[0];

    // Get the view details for the selected view configuration
    m_viewConfigurationViews = m_kdxrSystem->views(m_selectedViewConfiguration);

    // Check which versions of the graphics API are supported by the OpenXR runtime
    m_kdxrSystem->setGraphicsApi(m_api.get());
    auto graphicsRequirements = m_kdxrSystem->graphicsRequirements();
    SPDLOG_LOGGER_INFO(m_logger, "Minimum Vulkan API Version: {}", KDXr::getVersionAsString(graphicsRequirements.minApiVersionSupported));
    SPDLOG_LOGGER_INFO(m_logger, "Maximum Vulkan API Version: {}", KDXr::getVersionAsString(graphicsRequirements.maxApiVersionSupported));

    // Request an instance of the api with whatever layers and extensions we wish to request.
    const auto requiredGraphicsInstanceExtensions = m_kdxrSystem->requiredGraphicsInstanceExtensions();
    for (auto &requiredGraphicsInstanceExtension : requiredGraphicsInstanceExtensions) {
        SPDLOG_LOGGER_INFO(m_logger, "Requesting Vulkan Instance Extension: {}", requiredGraphicsInstanceExtension);
    }
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->applicationName(),
        .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0),
        .extensions = requiredGraphicsInstanceExtensions
    };
    m_instance = m_api->createInstance(instanceOptions);

    // Find which Adapter we should use for the given XR system
    Adapter *selectedAdapter = m_kdxrSystem->requiredGraphicsAdapter(m_instance);
    if (!selectedAdapter) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find required Vulkan Adapter.");
        throw std::runtime_error("Failed to find required Vulkan Adapter.");
    }
    const auto apiVersion = selectedAdapter->properties().apiVersion;
    SPDLOG_LOGGER_INFO(m_logger, "Graphics API Version: {}.{}.{}",
                       KDGPU_API_VERSION_MAJOR(apiVersion),
                       KDGPU_API_VERSION_MINOR(apiVersion),
                       KDGPU_API_VERSION_PATCH(apiVersion));

    // Request a device of the api with whatever layers and extensions we wish to request.
    const auto requiredGraphicsDeviceExtensions = m_kdxrSystem->requiredGraphicsDeviceExtensions();
    for (auto &requiredGraphicsDeviceExtension : requiredGraphicsDeviceExtensions) {
        SPDLOG_LOGGER_INFO(m_logger, "Requesting Vulkan Device Extension: {}", requiredGraphicsDeviceExtension);
    }
    DeviceOptions deviceOptions = {
        .extensions = requiredGraphicsDeviceExtensions,
        .requestedFeatures = selectedAdapter->features()
    };
    m_device = selectedAdapter->createDevice(deviceOptions);
    m_queue = m_device.queues()[0];

    // Create the XR session and track the state changes.
    m_kdxrSession = m_kdxrSystem->createSession({ .graphicsApi = m_api.get(), .device = m_device });
    m_kdxrSession.running.valueChanged().connect([this](bool running) {
        SPDLOG_LOGGER_INFO(m_logger, "Session Running: {}", running);
    });

    // Create a reference space - default to local space
    m_kdxrReferenceSpace = m_kdxrSession.createReferenceSpace();

    // Query the set of supported swapchain formats and select the color and depth formats to use
    std::span<const KDGpu::Format> supportedSwapchainFormats = m_kdxrSession.supportedSwapchainFormats();
    for (const auto &supportedSwapchainFormat : supportedSwapchainFormats) {
        SPDLOG_LOGGER_INFO(m_logger, "Supported Swapchain Format: {}", static_cast<int64_t>(supportedSwapchainFormat));
    }

    m_colorSwapchainFormat = m_kdxrSession.selectSwapchainFormat(m_applicationColorSwapchainFormats);
    if (m_colorSwapchainFormat == Format::UNDEFINED) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported SwapchainFormat.");
        throw std::runtime_error("Failed to find a supported color swapchain format.");
    }

    m_depthSwapchainFormat = m_kdxrSession.selectSwapchainFormat(m_applicationDepthSwapchainFormats);
    if (m_depthSwapchainFormat == Format::UNDEFINED) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported SwapchainFormat.");
        throw std::runtime_error("Failed to find a supported depth swapchain format.");
    }

    // TODO: Handle multiview rendering option
    // Create color and depth swapchains for each view
    const uint32_t viewCount = m_viewConfigurationViews.size();
    m_colorSwapchains.resize(viewCount);
    m_depthSwapchains.resize(viewCount);

    for (size_t i = 0; i < viewCount; ++i) {
        // Color swapchain and texture views
        auto &colorSwapchain = m_colorSwapchains[i];
        colorSwapchain.swapchain = m_kdxrSession.createSwapchain({ .format = m_colorSwapchainFormat,
                                                                   .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::ColorAttachmentBit,
                                                                   .width = m_viewConfigurationViews[i].recommendedTextureWidth,
                                                                   .height = m_viewConfigurationViews[i].recommendedTextureHeight,
                                                                   .sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount });
        const auto &textures = colorSwapchain.swapchain.textures();
        const auto textureCount = textures.size();
        colorSwapchain.textureViews.reserve(textureCount);
        for (size_t j = 0; j < textureCount; ++j)
            colorSwapchain.textureViews.emplace_back(textures[j].createView());

        // Depth swapchain and texture views
        auto &depthSwapchain = m_depthSwapchains[i];
        depthSwapchain.swapchain = m_kdxrSession.createSwapchain({ .format = m_depthSwapchainFormat,
                                                                   .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::DepthStencilAttachmentBit,
                                                                   .width = m_viewConfigurationViews[i].recommendedTextureWidth,
                                                                   .height = m_viewConfigurationViews[i].recommendedTextureHeight,
                                                                   .sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount });
        const auto &depthTextures = depthSwapchain.swapchain.textures();
        const auto depthTextureCount = depthTextures.size();
        depthSwapchain.textureViews.reserve(depthTextureCount);
        for (size_t j = 0; j < depthTextureCount; ++j)
            depthSwapchain.textureViews.emplace_back(depthTextures[j].createView());
    }

    // Create quad color and depth swapchains
    m_quadColorSwapchain.swapchain = m_kdxrSession.createSwapchain({ .format = m_colorSwapchainFormat,
                                                                     .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::ColorAttachmentBit,
                                                                     .width = m_quadSize.width,
                                                                     .height = m_quadSize.height,
                                                                     .sampleCount = 1 });
    const auto &quadTextures = m_quadColorSwapchain.swapchain.textures();
    const auto quadTextureCount = quadTextures.size();
    m_quadColorSwapchain.textureViews.reserve(quadTextureCount);
    for (size_t j = 0; j < quadTextureCount; ++j)
        m_quadColorSwapchain.textureViews.emplace_back(quadTextures[j].createView());

    m_quadDepthSwapchain.swapchain = m_kdxrSession.createSwapchain({ .format = m_depthSwapchainFormat,
                                                                     .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::DepthStencilAttachmentBit,
                                                                     .width = m_quadSize.width,
                                                                     .height = m_quadSize.height,
                                                                     .sampleCount = 1 });
    const auto &quadDepthTextures = m_quadDepthSwapchain.swapchain.textures();
    const auto quadDepthTextureCount = quadDepthTextures.size();
    m_quadDepthSwapchain.textureViews.reserve(quadDepthTextureCount);
    for (size_t j = 0; j < quadDepthTextureCount; ++j)
        m_quadDepthSwapchain.textureViews.emplace_back(quadDepthTextures[j].createView());

    // Initialize the ImGui overlay
    recreateImGuiOverlay();

    // Delegate to subclass to initialize scene
    initializeScene();
}

void XrExampleEngineLayer::onDetached()
{
    m_imguiOverlay = {};
    m_quadColorSwapchain.textureViews.clear();
    m_quadColorSwapchain.swapchain = {};
    m_quadDepthSwapchain.textureViews.clear();
    m_quadDepthSwapchain.swapchain = {};
    m_colorSwapchains.clear();
    m_depthSwapchains.clear();
    m_kdxrReferenceSpace = {};
    m_kdxrSession = {};
    m_queue = {};
    m_device = {};
    m_instance = {};
    m_kdxrInstance = {};
}

void XrExampleEngineLayer::update()
{
    // Release any staging buffers we are done with
    releaseStagingBuffers();

    // TODO: Handle events and pass on to ImGui as needed
    // Update the ImGui overlay
    updateImGuiOverlay();

    // Process XR events
    m_kdxrInstance.processEvents();

    if (!m_kdxrSession.running())
        return;

    // Get timing information from the XR runtime and throttle the frame rate
    const KDXr::FrameState frameState = m_kdxrSession.waitForFrame();
    if (frameState.waitFrameResult != KDXr::WaitFrameResult::Success) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to wait for frame.");
        return;
    }

    // Inform the XR compositor that we are beginning to render the frame
    if (m_kdxrSession.beginFrame() != KDXr::BeginFrameResult::Success) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to begin frame.");
        return;
    }

    // Reset the compositor layers
    m_compositorLayers.clear();

    if (m_kdxrSession.isActive() && frameState.shouldRender) {
        // For now, we will use only a single projection layer. Later we can extend this to support multiple compositor layer types
        // in any configuration. At this time we assume the scene in the subclass is the only thing to be composited.

        // Locate the views from the view configuration within the (reference) space at the display time.
        const auto locateViewsOptions = KDXr::LocateViewsOptions{
            .displayTime = frameState.predictedDisplayTime,
            .referenceSpace = m_kdxrReferenceSpace
        };
        KDXr::ViewState viewState{ .views = m_views };
        const auto result = m_kdxrSession.locateViews(locateViewsOptions, viewState);
        if (result != KDXr::LocateViewsResult::Success) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to locate views.");
        } else {
            // Call updateScene() function to update scene state.
            updateScene();

            // Render the projection layer
            m_projectionLayerViews.resize(viewState.viewCount);

            for (m_currentViewIndex = 0; m_currentViewIndex < viewState.viewCount; ++m_currentViewIndex) {
                // Acquire and wait for the next swapchain textures to become available for the color and depth swapchains
                KDXr::SwapchainInfo &colorSwapchainInfo = m_colorSwapchains[m_currentViewIndex];
                KDXr::SwapchainInfo &depthSwapchainInfo = m_depthSwapchains[m_currentViewIndex];

                colorSwapchainInfo.swapchain.getNextTextureIndex(m_currentColorImageIndex);
                depthSwapchainInfo.swapchain.getNextTextureIndex(m_currentDepthImageIndex);

                colorSwapchainInfo.swapchain.waitForTexture();
                depthSwapchainInfo.swapchain.waitForTexture();

                // Update the projection layer view for the current view
                // clang-format off
                m_projectionLayerViews[m_currentViewIndex] = {
                    .pose = m_views[m_currentViewIndex].pose,
                    .fieldOfView = m_views[m_currentViewIndex].fieldOfView,
                    .swapchainSubTexture = {
                        .swapchain = colorSwapchainInfo.swapchain,
                        .rect = {
                            .offset = { .x = 0, .y = 0 },
                            .extent = {
                                .width = m_viewConfigurationViews[m_currentViewIndex].recommendedTextureWidth,
                                .height = m_viewConfigurationViews[m_currentViewIndex].recommendedTextureHeight
                            }
                        }
                    }
                };
                // clang-format on

                // Call subclass renderView() function to record and submit drawing commands for the current view
                renderView();

                // Give the swapchain textures back to the XR runtime, allowing the compositor to use the image.
                colorSwapchainInfo.swapchain.releaseTexture();
                depthSwapchainInfo.swapchain.releaseTexture();
            }

            // Set up the projection layer
            m_projectionLayers[0] = {
                .type = KDXr::CompositionLayerType::Projection,
                .referenceSpace = m_kdxrReferenceSpace,
                .flags = KDXr::CompositionLayerFlagBits::BlendTextureSourceAlphaBit | KDXr::CompositionLayerFlagBits::CorrectChromaticAberrationBit,
                .views = m_projectionLayerViews // All views - adjust to relevant subset if we add support for multiple projection layers
            };
            m_compositorLayers.push_back(reinterpret_cast<KDXr::CompositionLayer *>(&m_projectionLayers[0]));

            // Render the quad layer
            // Acquire and wait for the next swapchain textures to become available for the color and depth swapchains
            m_quadColorSwapchain.swapchain.getNextTextureIndex(m_currentColorImageIndex);
            m_quadDepthSwapchain.swapchain.getNextTextureIndex(m_currentDepthImageIndex);

            m_quadColorSwapchain.swapchain.waitForTexture();
            m_quadDepthSwapchain.swapchain.waitForTexture();

            // Call subclass renderQuad() function to record and submit drawing commands for the quad
            renderQuad();

            // Give the swapchain textures back to the XR runtime, allowing the compositor to use the image.
            m_quadColorSwapchain.swapchain.releaseTexture();
            m_quadDepthSwapchain.swapchain.releaseTexture();

            // Set up the quad layer
            // clang-format off
            m_quadLayers[0] = {
                .type = KDXr::CompositionLayerType::Quad,
                .referenceSpace = m_kdxrReferenceSpace,
                .flags = KDXr::CompositionLayerFlagBits::BlendTextureSourceAlphaBit | KDXr::CompositionLayerFlagBits::CorrectChromaticAberrationBit,
                .eyeVisibility = KDXr::EyeVisibility::Both,
                .swapchainSubTexture = {
                    .swapchain = m_quadColorSwapchain.swapchain,
                    .rect = {
                        .offset = { .x = 0, .y = 0 },
                        .extent = {
                            .width = m_quadSize.width,
                            .height = m_quadSize.height
                        }
                    }
                },
                .pose = m_quadPose,
                .size = m_quadWorldSize
            };
            // clang-format on
            m_compositorLayers.push_back(reinterpret_cast<KDXr::CompositionLayer *>(&m_quadLayers[0]));
        }
    }

    // Inform the XR compositor that we are done rendering the frame. We must specify the display time,
    // environment blend mode, and the list of layers to compose.
    const auto endFrameOptions = KDXr::EndFrameOptions{
        .displayTime = frameState.predictedDisplayTime,
        .environmentBlendMode = m_selectedEnvironmentBlendMode,
        .layers = m_compositorLayers
    };
    if (m_kdxrSession.endFrame(endFrameOptions) != KDXr::EndFrameResult::Success) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to end frame.");
    }
}

void XrExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
}

void XrExampleEngineLayer::onInstanceLost()
{
    SPDLOG_LOGGER_ERROR(m_logger, "Instance Lost.");
    // TODO: Gracefully handle shutting down the application
}

void XrExampleEngineLayer::uploadBufferData(const BufferUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadBufferData(options));
}

void XrExampleEngineLayer::uploadTextureData(const TextureUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadTextureData(options));
}

void XrExampleEngineLayer::releaseStagingBuffers()
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

void XrExampleEngineLayer::drawImGuiOverlay(ImGuiContext *ctx)
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
    ImGui::End();

    for (const auto &func : m_imGuiOverlayDrawFunctions)
        func(ctx);
}

void XrExampleEngineLayer::renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    const Extent2D extent{ m_quadSize.width, m_quadSize.height };
    m_imguiOverlay->render(recorder, extent, inFlightIndex);
}

void XrExampleEngineLayer::registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func)
{
    m_imGuiOverlayDrawFunctions.push_back(func);
}

void XrExampleEngineLayer::clearImGuiOverlayDrawFunctions()
{
    m_imGuiOverlayDrawFunctions.clear();
}

void XrExampleEngineLayer::recreateImGuiOverlay()
{
    m_imguiOverlay = std::make_unique<ImGuiItem>(&m_device, &m_queue);
    m_imguiOverlay->initialize(1.0f, m_samples.get(), m_colorSwapchainFormat, m_depthSwapchainFormat);
}

void XrExampleEngineLayer::updateImGuiOverlay()
{
    ImGuiContext *context = m_imguiOverlay->context();
    ImGui::SetCurrentContext(context);

    // Set frame time and display size.
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = engine()->deltaTimeSeconds();
    io.DisplaySize = ImVec2(static_cast<float>(m_quadSize.width), static_cast<float>(m_quadSize.height));

    // Call our imgui drawing function
    ImGui::NewFrame();
    drawImGuiOverlay(context);

    // Process the ImGui drawing functions to generate geometry and commands. The actual buffers will be updated
    // and commands translated by the ImGuiRenderer later in the frame.
    ImGui::Render();
}

} // namespace KDGpuExample
