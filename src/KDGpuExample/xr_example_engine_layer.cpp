/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_example_engine_layer.h"

#include <KDGpuExample/engine.h>
#include <KDXr/utils/formatters.h>
#include <KDGpu/texture_options.h>
#include <KDGui/gui_application.h>
#include <KDUtils/logging.h>

#include <assert.h>

#ifdef PLATFORM_ANDROID
#include <KDGui/platform/android/android_platform_integration.h>
#endif

namespace KDGpuExample {

XrExampleEngineLayer::XrExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
    , m_xrApi(std::make_unique<KDXr::OpenXrApi>())
{
    m_logger = KDUtils::Logger::logger("engine", spdlog::level::info);

#if defined(PLATFORM_ANDROID)
    KDXr::OpenXrApi::initializeAndroid(KDGui::AndroidPlatformIntegration::s_androidApp);
#endif
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
        .extensions = { XR_EXT_DEBUG_UTILS_EXTENSION_NAME,
                        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME,
                        XR_KHR_COMPOSITION_LAYER_CYLINDER_EXTENSION_NAME }
    };
    m_xrInstance = m_xrApi->createInstance(xrInstanceOptions);
    m_xrInstance.instanceLost.connect(&XrExampleEngineLayer::onInstanceLost, this);
    m_xrInstance.interactionProfileChanged.connect(&XrExampleEngineLayer::onInteractionProfileChanged, this);
    const auto properties = m_xrInstance.properties();
    SPDLOG_LOGGER_INFO(m_logger, "XR Runtime: {}", properties.runtimeName);
    SPDLOG_LOGGER_INFO(m_logger, "XR Runtime Version: {}", KDXr::getVersionAsString(properties.runtimeVersion));

    m_system = m_xrInstance.system();
    if (!m_system) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find an XR system. Please ensure that your XR device is connected and powered on.");
        KDGui::GuiApplication::instance()->quit();
        return;
    }
    const auto systemProperties = m_system->properties();

    // Pick the first application supported View Configuration Type supported by the hardware.
    m_selectedViewConfiguration = m_system->selectViewConfiguration(m_applicationViewConfigurations);
    if (m_selectedViewConfiguration == KDXr::ViewConfigurationType::MaxEnum) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported ViewConfigurationType.");
        throw std::runtime_error("Failed to find a supported ViewConfigurationType.");
    }

    // We will just use the first environment blend mode supported by the system
    m_selectedEnvironmentBlendMode = m_system->environmentBlendModes(m_selectedViewConfiguration)[0];

    // Get the view details for the selected view configuration
    m_viewConfigurationViews = m_system->views(m_selectedViewConfiguration);

    // Check which versions of the graphics API are supported by the OpenXR runtime
    m_system->setGraphicsApi(m_api.get());
    auto graphicsRequirements = m_system->graphicsRequirements();
    SPDLOG_LOGGER_INFO(m_logger, "Minimum Vulkan API Version: {}", KDXr::getVersionAsString(graphicsRequirements.minApiVersionSupported));
    SPDLOG_LOGGER_INFO(m_logger, "Maximum Vulkan API Version: {}", KDXr::getVersionAsString(graphicsRequirements.maxApiVersionSupported));

    // Request an instance of the api with whatever layers and extensions we wish to request.
    const auto requiredGraphicsInstanceExtensions = m_system->requiredGraphicsInstanceExtensions();
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
    Adapter *selectedAdapter = m_system->requiredGraphicsAdapter(m_instance);
    if (!selectedAdapter) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find required Vulkan Adapter.");
        KDGui::GuiApplication::instance()->quit();
        return;
    }

    // Request a device of the api with whatever layers and extensions we wish to request.
    const auto requiredGraphicsDeviceExtensions = m_system->requiredGraphicsDeviceExtensions();
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
    m_session = m_system->createSession({ .graphicsApi = m_api.get(), .device = m_device });
    m_session.running.valueChanged().connect([this](bool running) {
        SPDLOG_LOGGER_INFO(m_logger, "Session Running: {}", running);
    });
    m_session.state.valueAboutToChange().connect(&XrExampleEngineLayer::onSessionStateChanged, this);

    // Create a reference space - default to local space
    m_referenceSpace = m_session.createReferenceSpace();

    // Query the set of supported swapchain formats and select the color and depth formats to use
    m_colorSwapchainFormat = m_session.selectSwapchainFormat(m_applicationColorSwapchainFormats);
    if (m_colorSwapchainFormat == Format::UNDEFINED) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported SwapchainFormat.");
        KDGui::GuiApplication::instance()->quit();
        return;
    }
    m_depthSwapchainFormat = m_session.selectSwapchainFormat(m_applicationDepthSwapchainFormats);
    if (m_depthSwapchainFormat == Format::UNDEFINED) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported SwapchainFormat.");
        KDGui::GuiApplication::instance()->quit();
        return;
    }

    m_isInitialized = true;

    // Each of the compositor layer objects will be responsible for creating and managing its own swapchains
    // and any other resources they require.
}

void XrExampleEngineLayer::onDetached()
{
    m_compositorLayerObjects.clear();
    m_referenceSpace = {};
    m_session = {};
    m_queue = {};
    m_device = {};
    m_instance = {};
    m_xrInstance = {};
    m_isInitialized = false;
}

void XrExampleEngineLayer::update()
{
    // Release any staging buffers we are done with
    releaseStagingBuffers();

    // Process XR events
    m_xrInstance.processEvents();

    if (!m_session.running())
        return;

    // Get timing information from the XR runtime and throttle the frame rate
    const KDXr::FrameState frameState = m_session.waitForFrame();
    if (frameState.waitFrameResult != KDXr::WaitFrameResult::Success) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to wait for frame.");
        return;
    }

    // Inform the XR compositor that we are beginning to render the frame
    if (m_session.beginFrame() != KDXr::BeginFrameResult::Success) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to begin frame.");
        return;
    }

    // Reset the compositor layers that we will be submitting to the compositor
    m_compositorLayers.clear();

    if (m_session.isActive() && frameState.shouldRender) {
        pollActions(frameState.predictedDisplayTime);

        // Ask each compositor layer object to update its state, render and prepare its composition layer.
        // If the compositor layer object has nothing to render or it cannot locate the views, it will return
        // false and we will not add its composition layer to the list of layers to be submitted to the compositor.
        for (auto &compositorLayerObject : m_compositorLayerObjects) {
            if (compositorLayerObject->update(frameState))
                m_compositorLayers.push_back(compositorLayerObject->compositionLayer());
        }
    }

    // Inform the XR compositor that we are done rendering the frame. We must specify the display time,
    // environment blend mode, and the list of layers to compose.
    const auto endFrameOptions = KDXr::EndFrameOptions{
        .displayTime = frameState.predictedDisplayTime,
        .environmentBlendMode = m_selectedEnvironmentBlendMode,
        .layers = m_compositorLayers
    };
    if (m_session.endFrame(endFrameOptions) != KDXr::EndFrameResult::Success) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to end frame.");
    }
}

void XrExampleEngineLayer::pollActions(KDXr::Time predictedDisplayTime)
{
    // Do nothing by default. Subclasses can override this to poll actions and react as needed.
}

void XrExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
}

void XrExampleEngineLayer::onSessionStateChanged(KDXr::SessionState oldState, KDXr::SessionState newState)
{
    SPDLOG_LOGGER_INFO(m_logger, "Session State Changed: {} -> {}", oldState, newState);
    if (newState == KDXr::SessionState::Exiting)
        KDGui::GuiApplication::instance()->quit();
}

void XrExampleEngineLayer::onInstanceLost()
{
    SPDLOG_LOGGER_ERROR(m_logger, "Instance Lost.");
    // Try to gracefully handle shutting down the application
    KDGui::GuiApplication::instance()->quit();
}

void XrExampleEngineLayer::onInteractionProfileChanged()
{
    SPDLOG_LOGGER_INFO(m_logger, "Interaction Profile Changed.");
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

} // namespace KDGpuExample
