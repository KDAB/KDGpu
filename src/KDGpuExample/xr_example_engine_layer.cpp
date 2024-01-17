/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_example_engine_layer.h"

#include <KDGpuExample/engine.h>

#include <KDGpu/texture_options.h>

#include <KDGui/gui_application.h>

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
    const auto properties = m_kdxrInstance.properties();
    SPDLOG_LOGGER_INFO(m_logger, "OpenXR Runtime: {}", properties.runtimeName);
    SPDLOG_LOGGER_INFO(m_logger, "OpenXR API Version: {}.{}.{}",
                       KDXR_VERSION_MAJOR(properties.runtimeVersion),
                       KDXR_VERSION_MINOR(properties.runtimeVersion),
                       KDXR_VERSION_PATCH(properties.runtimeVersion));

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
    SPDLOG_LOGGER_INFO(m_logger, "Minimum Vulkan API Version: {}.{}.{}",
                       KDXR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported),
                       KDXR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported),
                       KDXR_VERSION_PATCH(graphicsRequirements.minApiVersionSupported));
    SPDLOG_LOGGER_INFO(m_logger, "Maximum Vulkan API Version: {}.{}.{}",
                       KDXR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported),
                       KDXR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported),
                       KDXR_VERSION_PATCH(graphicsRequirements.maxApiVersionSupported));

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

    // Create the XR session
    m_kdxrSession = m_kdxrSystem->createSession({ .graphicsApi = m_api.get(), .device = m_device });

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

    // TODO: Remove this temporary exposure of underlying OpenXR resources once KDXr is suitable for use.
    auto *openXrResourceManager = dynamic_cast<KDXr::OpenXrResourceManager *>(m_xrApi->resourceManager());
    assert(openXrResourceManager);

    for (size_t i = 0; i < viewCount; ++i) {
        // Color swapchain and texture views
        auto &colorSwapchain = m_colorSwapchains[i];
        colorSwapchain.swapchain = m_kdxrSession.createSwapchain({ .format = m_colorSwapchainFormat,
                                                                   .usage = KDXr::SwapchainUsageFlagBits::SampledBit | KDXr::SwapchainUsageFlagBits::ColorAttachmentBit,
                                                                   .width = m_viewConfigurationViews[i].recommendedTextureWidth,
                                                                   .height = m_viewConfigurationViews[i].recommendedTextureHeight,
                                                                   .sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount });
        colorSwapchain.xrSwapchain = openXrResourceManager->getSwapchain(colorSwapchain.swapchain.handle())->swapchain;
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
        depthSwapchain.xrSwapchain = openXrResourceManager->getSwapchain(depthSwapchain.swapchain.handle())->swapchain;
        const auto &depthTextures = depthSwapchain.swapchain.textures();
        const auto depthTextureCount = depthTextures.size();
        depthSwapchain.textureViews.reserve(depthTextureCount);
        for (size_t j = 0; j < depthTextureCount; ++j)
            depthSwapchain.textureViews.emplace_back(depthTextures[j].createView());
    }

    // TODO: Remove this temporary exposure of underlying OpenXR resources once KDXr is suitable for use.
    // It just allows us to use the raw C api for the stuff that is not implemented in KDXr yet.
    auto *openxrInstance = openXrResourceManager->getInstance(m_kdxrInstance.handle());
    assert(openxrInstance);
    m_xrInstance = openxrInstance->instance;
    auto *openxrSystem = openXrResourceManager->getSystem(m_kdxrSystem->handle());
    assert(openxrSystem);
    m_systemId = openxrSystem->system;
    auto *openXrSession = openXrResourceManager->getSession(m_kdxrSession.handle());
    m_xrSession = openXrSession->session;
    auto *openxrReferenceSpace = openXrResourceManager->getReferenceSpace(m_kdxrReferenceSpace.handle());
    m_xrReferenceSpace = openxrReferenceSpace->referenceSpace;

    // Delegate to subclass to initialize scene
    initializeScene();
}

void XrExampleEngineLayer::onDetached()
{
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

    pollXrEvents();

    if (!m_xrSessionRunning)
        return;

    // Get timing information from OpenXR
    XrFrameState frameState{ XR_TYPE_FRAME_STATE };
    XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
    if (xrWaitFrame(m_xrSession, &frameWaitInfo, &frameState) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to wait for frame.");
        return;
    }

    // Inform the OpenXR compositor that we are beginning to render the frame
    XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
    if (xrBeginFrame(m_xrSession, &frameBeginInfo) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to begin frame.");
        return;
    }

    // Start off with no layers to compose and set the predicted display time
    m_xrCompositorLayerInfo.reset(frameState.predictedDisplayTime);

    const bool sessionActive = (m_xrSessionState == XR_SESSION_STATE_SYNCHRONIZED || m_xrSessionState == XR_SESSION_STATE_VISIBLE || m_xrSessionState == XR_SESSION_STATE_FOCUSED);
    if (sessionActive && frameState.shouldRender) {
        // For now, we will use only a single projection layer. Later we can extend this to support multiple compositor layer types
        // in any configuration. At this time we assume the scene in the subclass is the only thing to be composited.

        // Locate the views from the view configuration within the (reference) space at the display time.
        std::vector<XrView> views(m_viewConfigurationViews.size(), { XR_TYPE_VIEW });

        XrViewState viewState{ XR_TYPE_VIEW_STATE }; // Contains information on whether the position and/or orientation is valid and/or tracked.
        XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
        viewLocateInfo.viewConfigurationType = static_cast<XrViewConfigurationType>(m_selectedViewConfiguration); // TODO: Add conversion helper function to KDXr
        viewLocateInfo.displayTime = m_xrCompositorLayerInfo.predictedDisplayTime;
        viewLocateInfo.space = m_xrReferenceSpace;
        uint32_t viewCount = 0;
        if (xrLocateViews(m_xrSession, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data()) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to locate views.");
        } else {
            // Store the XrView data for use in the subclass render functions
            for (uint32_t i = 0; i < viewCount; ++i) {
                // clang-format off
                m_views[i] = {
                    .pose = {
                        .orientation = glm::quat{
                            views[i].pose.orientation.w,
                            views[i].pose.orientation.x,
                            views[i].pose.orientation.y,
                            views[i].pose.orientation.z
                        },
                        .position = {
                            views[i].pose.position.x,
                            views[i].pose.position.y,
                            views[i].pose.position.z
                        }
                    },
                    .fieldOfView = {
                        .angleLeft = views[i].fov.angleLeft,
                        .angleRight = views[i].fov.angleRight,
                        .angleUp = views[i].fov.angleUp,
                        .angleDown = views[i].fov.angleDown
                    }
                };
                // clang-format on
            }

            // Call updateScene() function to update scene state.
            updateScene();

            m_xrCompositorLayerInfo.layerProjectionViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });

            for (m_currentViewIndex = 0; m_currentViewIndex < viewCount; ++m_currentViewIndex) {
                // Acquire and wait for the swapchain images to become available for the color and depth swapchains
                KDXrSwapchainInfo &colorSwapchainInfo = m_colorSwapchains[m_currentViewIndex];
                KDXrSwapchainInfo &depthSwapchainInfo = m_depthSwapchains[m_currentViewIndex];

                colorSwapchainInfo.swapchain.getNextTextureIndex(m_currentColorImageIndex);
                depthSwapchainInfo.swapchain.getNextTextureIndex(m_currentDepthImageIndex);

                XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
                waitInfo.timeout = XR_INFINITE_DURATION;
                if (xrWaitSwapchainImage(colorSwapchainInfo.xrSwapchain, &waitInfo) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to wait for Image from the Color Swapchain");
                }
                if (xrWaitSwapchainImage(depthSwapchainInfo.xrSwapchain, &waitInfo) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to wait for Image from the Depth Swapchain");
                }

                const uint32_t &width = m_viewConfigurationViews[m_currentViewIndex].recommendedTextureWidth;
                const uint32_t &height = m_viewConfigurationViews[m_currentViewIndex].recommendedTextureHeight;

                m_xrCompositorLayerInfo.layerProjectionViews[m_currentViewIndex].pose = views[m_currentViewIndex].pose;
                m_xrCompositorLayerInfo.layerProjectionViews[m_currentViewIndex].fov = views[m_currentViewIndex].fov;
                m_xrCompositorLayerInfo.layerProjectionViews[m_currentViewIndex].subImage.swapchain = colorSwapchainInfo.xrSwapchain;
                m_xrCompositorLayerInfo.layerProjectionViews[m_currentViewIndex].subImage.imageRect = { 0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height) };
                m_xrCompositorLayerInfo.layerProjectionViews[m_currentViewIndex].subImage.imageArrayIndex = 0;

                // Call subclass renderView() function to record and submit drawing commands for the current view
                renderView();

                // Give the swapchain image back to OpenXR, allowing the compositor to use the image.
                XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
                if (xrReleaseSwapchainImage(colorSwapchainInfo.xrSwapchain, &releaseInfo) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to release Image back to the Color Swapchain");
                }
                if (xrReleaseSwapchainImage(depthSwapchainInfo.xrSwapchain, &releaseInfo) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to release Image back to the Depth Swapchain");
                }
            }

            // Set up the projection layer
            XrCompositionLayerProjection projectionLayer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
            projectionLayer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
            projectionLayer.space = m_xrReferenceSpace;
            projectionLayer.viewCount = m_xrCompositorLayerInfo.layerProjectionViews.size();
            projectionLayer.views = m_xrCompositorLayerInfo.layerProjectionViews.data();

            m_xrCompositorLayerInfo.layerProjections.emplace_back(projectionLayer);
            m_xrCompositorLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader *>(&m_xrCompositorLayerInfo.layerProjections.back()));
        }
    }

    // Inform the OpenXR compositor that we are done rendering the frame. We must specify the display time,
    // environment blend mode, and the list of layers to compose.
    XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    frameEndInfo.environmentBlendMode = static_cast<XrEnvironmentBlendMode>(m_selectedEnvironmentBlendMode); // TODO: Add conversion helper function to KDXr
    frameEndInfo.layerCount = static_cast<uint32_t>(m_xrCompositorLayerInfo.layers.size());
    frameEndInfo.layers = m_xrCompositorLayerInfo.layers.data();
    if (xrEndFrame(m_xrSession, &frameEndInfo) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to end frame.");
        return;
    }
}

void XrExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
}

void XrExampleEngineLayer::pollXrEvents()
{
    XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
    auto XrPollEvents = [&]() -> bool {
        eventData = { XR_TYPE_EVENT_DATA_BUFFER };
        return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
    };

    while (XrPollEvents()) {
        switch (eventData.type) {
        case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
            SPDLOG_LOGGER_WARN(m_logger, "OpenXR Events Lost.");
            break;
        }

        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
            SPDLOG_LOGGER_WARN(m_logger, "OpenXR Instance Loss Pending.");
            m_xrSessionRunning = false;
            break;
        }

        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
            SPDLOG_LOGGER_INFO(m_logger, "OpenXR Interaction Profile Changed.");
            // TODO: Handle this event
            break;
        }

        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
            SPDLOG_LOGGER_INFO(m_logger, "OpenXR Reference Space Change Pending.");
            // TODO: Handle this event
            break;
        }

        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            const auto *sessionStateChanged = reinterpret_cast<const XrEventDataSessionStateChanged *>(&eventData);

            if (sessionStateChanged->session != m_xrSession) {
                SPDLOG_LOGGER_WARN(m_logger, "OpenXR Session State Changed for unknown session.");
                break;
            }

            switch (sessionStateChanged->state) {
            case XR_SESSION_STATE_READY: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Ready.");
                XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
                sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                if (xrBeginSession(m_xrSession, &sessionBeginInfo) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to begin OpenXR Session.");
                    return;
                }
                m_xrSessionRunning = true;
                break;
            }

            case XR_SESSION_STATE_SYNCHRONIZED: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Synchronized.");
                break;
            }

            case XR_SESSION_STATE_VISIBLE: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Visible.");
                break;
            }

            case XR_SESSION_STATE_FOCUSED: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Focused.");
                break;
            }

            case XR_SESSION_STATE_STOPPING: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Stopping.");
                m_xrSessionRunning = false;
                break;
            }

            case XR_SESSION_STATE_LOSS_PENDING: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Loss Pending.");
                m_xrSessionRunning = false;
                // TODO: Handle this event and exit the application or try to recreate the XrInstance and XrSession.
                break;
            }

            default: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Unknown.");
                break;
            }
            }

            m_xrSessionState = sessionStateChanged->state;
        }
        }
    }
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
