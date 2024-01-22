/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_session.h"

#include <KDXr/session.h>
#include <KDXr/openxr/openxr_enums.h>
#include <KDXr/openxr/openxr_resource_manager.h>
#include <KDXr/utils/formatters.h>
#include <KDXr/utils/logging.h>

#include <KDGpu/graphics_api.h>

namespace {

KDXr::Pose xrPoseToPose(const XrPosef &xrPose)
{
    return KDXr::Pose{
        .orientation = KDXr::Quaternion{ xrPose.orientation.x, xrPose.orientation.y, xrPose.orientation.z, xrPose.orientation.w },
        .position = KDXr::Vector3{ xrPose.position.x, xrPose.position.y, xrPose.position.z }
    };
}

XrPosef poseToXrPose(const KDXr::Pose &pose)
{
    return XrPosef{
        .orientation = XrQuaternionf{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w },
        .position = XrVector3f{ pose.position.x, pose.position.y, pose.position.z }
    };
}

KDXr::FieldOfView xrFovToFov(const XrFovf &xrFov)
{
    return KDXr::FieldOfView{
        .angleLeft = xrFov.angleLeft,
        .angleRight = xrFov.angleRight,
        .angleUp = xrFov.angleUp,
        .angleDown = xrFov.angleDown
    };
}

XrFovf fovToXrFov(const KDXr::FieldOfView &fov)
{
    return XrFovf{
        .angleLeft = fov.angleLeft,
        .angleRight = fov.angleRight,
        .angleUp = fov.angleUp,
        .angleDown = fov.angleDown
    };
}

XrRect2Di rect2DToXrRecti(const KDGpu::Rect2D &rect)
{
    return XrRect2Di{
        .offset = XrOffset2Di{ rect.offset.x, rect.offset.y },
        .extent = XrExtent2Di{ static_cast<int32_t>(rect.extent.width), static_cast<int32_t>(rect.extent.height) }
    };
}

} // namespace

namespace KDXr {

OpenXrSession::OpenXrSession(OpenXrResourceManager *_openxrResourceManager,
                             XrSession _session,
                             const Handle<System_t> _systemHandle,
                             KDGpu::GraphicsApi *_graphicsApi,
                             KDGpu::Handle<KDGpu::Device_t> _device,
                             uint32_t queueIndex) noexcept
    : ApiSession()
    , openxrResourceManager(_openxrResourceManager)
    , session(_session)
    , systemHandle(_systemHandle)
    , graphicsApi(_graphicsApi)
    , deviceHandle(_device)
    , queueIndex(queueIndex)
{
}

void OpenXrSession::initialize(Session *_frontendSession)
{
    frontendSession = _frontendSession;
}

std::vector<KDGpu::Format> OpenXrSession::supportedSwapchainFormats() const
{
    // Query the number of swapchain formats supported by the system
    uint32_t swapchainFormatCount = 0;
    if (xrEnumerateSwapchainFormats(session, 0, &swapchainFormatCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate SwapchainFormats.");
        return {};
    }

    // Query the swapchain formats supported by the system
    std::vector<int64_t> xrSwapchainFormats;
    xrSwapchainFormats.resize(swapchainFormatCount);
    if (xrEnumerateSwapchainFormats(session, swapchainFormatCount, &swapchainFormatCount, xrSwapchainFormats.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate SwapchainFormats.");
        return {};
    }

    // Note: KDGpu formats have the same value as the Vulkan formats. So if we are using the Vulkan backend we can just
    // use the KDGpu formats directly. If we are using the Metal or DX12 backend we need to convert the KDGpu formats to the
    // Metal or DX12 formats.
    if (graphicsApi->api() == KDGpu::GraphicsApi::Api::Vulkan) {
        std::vector<KDGpu::Format> formats;
        formats.reserve(xrSwapchainFormats.size());
        for (const auto &xrSwapchainFormat : xrSwapchainFormats) {
            formats.push_back(static_cast<KDGpu::Format>(xrSwapchainFormat));
        }
        return formats;
    } else {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSession::supportedSwapchainFormats(). Unsupported graphics API.");
        return {};
    }
}

FrameState OpenXrSession::waitForFrame()
{
    XrFrameState frameState{ XR_TYPE_FRAME_STATE };
    XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
    const auto result = xrWaitFrame(session, &frameWaitInfo, &frameState);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to wait for frame.");
        return {};
    }

    return FrameState{
        .waitFrameResult = static_cast<WaitFrameResult>(result),
        .predictedDisplayTime = frameState.predictedDisplayTime,
        .predictedDisplayPeriod = frameState.predictedDisplayPeriod,
        .shouldRender = static_cast<bool>(frameState.shouldRender)
    };
}

BeginFrameResult OpenXrSession::beginFrame()
{
    XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
    const auto result = xrBeginFrame(session, &frameBeginInfo);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to begin frame.");
    }
    return static_cast<BeginFrameResult>(result);
}

EndFrameResult OpenXrSession::endFrame(const EndFrameOptions &options)
{
    // Create the corresponding OpenXR composition layer structures
    const auto layerCount = options.layers.size();
    xrLayers.resize(layerCount);

    // Clear the various types of projection layer containers. The first time we call endFrame(),
    // the vectors will begin to grow to the size of the number of layers. After that, they will
    // maintain a high-water mark of the number of layers of each type.
    xrLayerProjections.clear();
    xrLayerProjectionViews.clear();
    xrLayerQuads.clear();

    for (size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        switch (options.layers[layerIndex]->type) {
        case CompositionLayerType::Projection: {
            auto &projectionLayer = reinterpret_cast<ProjectionLayer &>(*options.layers[layerIndex]);
            const auto viewCount = projectionLayer.views.size();

            for (size_t viewIndex = 0; viewIndex < viewCount; ++viewIndex) {
                auto openxrSwapchain = openxrResourceManager->getSwapchain(projectionLayer.views[viewIndex].swapchainSubTexture.swapchain);
                assert(openxrSwapchain);

                xrLayerProjectionViews.push_back({ XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });
                auto &projectionView = xrLayerProjectionViews.back();
                projectionView.pose = poseToXrPose(projectionLayer.views[viewIndex].pose);
                projectionView.fov = fovToXrFov(projectionLayer.views[viewIndex].fieldOfView);
                projectionView.subImage.swapchain = openxrSwapchain->swapchain;
                projectionView.subImage.imageRect = rect2DToXrRecti(projectionLayer.views[viewIndex].swapchainSubTexture.rect);
                projectionView.subImage.imageArrayIndex = projectionLayer.views[viewIndex].swapchainSubTexture.arrayIndex;
            }

            auto openxrReferenceSpace = openxrResourceManager->getReferenceSpace(projectionLayer.referenceSpace);
            assert(openxrReferenceSpace);

            xrLayerProjections.push_back({ XR_TYPE_COMPOSITION_LAYER_PROJECTION });
            auto &projectionLayerProjection = xrLayerProjections.back();
            projectionLayerProjection.layerFlags = compositionLayerFlagsToXrCompositionLayerFlags(projectionLayer.flags);
            projectionLayerProjection.space = openxrReferenceSpace->referenceSpace;
            projectionLayerProjection.viewCount = static_cast<uint32_t>(viewCount);
            projectionLayerProjection.views = xrLayerProjectionViews.data() + xrLayerProjectionViews.size() - viewCount;

            xrLayers[layerIndex] = reinterpret_cast<XrCompositionLayerBaseHeader *>(&projectionLayerProjection);

            break;
        }

        case CompositionLayerType::Quad: {
            auto &quadLayer = reinterpret_cast<QuadLayer &>(*options.layers[layerIndex]);

            auto openxrSwapchain = openxrResourceManager->getSwapchain(quadLayer.swapchainSubTexture.swapchain);
            assert(openxrSwapchain);
            auto openxrReferenceSpace = openxrResourceManager->getReferenceSpace(quadLayer.referenceSpace);
            assert(openxrReferenceSpace);

            xrLayerQuads.push_back({ XR_TYPE_COMPOSITION_LAYER_QUAD });
            auto &quadLayerQuad = xrLayerQuads.back();
            quadLayerQuad.layerFlags = compositionLayerFlagsToXrCompositionLayerFlags(quadLayer.flags);
            quadLayerQuad.space = openxrReferenceSpace->referenceSpace;
            quadLayerQuad.eyeVisibility = eyeVisibilityToXrEyeVisibility(quadLayer.eyeVisibility);
            quadLayerQuad.subImage.swapchain = openxrSwapchain->swapchain;
            quadLayerQuad.subImage.imageRect = rect2DToXrRecti(quadLayer.swapchainSubTexture.rect);
            quadLayerQuad.subImage.imageArrayIndex = quadLayer.swapchainSubTexture.arrayIndex;
            quadLayerQuad.pose = poseToXrPose(quadLayer.pose);
            quadLayerQuad.size = XrExtent2Df{ quadLayer.size.width, quadLayer.size.height };

            xrLayers[layerIndex] = reinterpret_cast<XrCompositionLayerBaseHeader *>(&quadLayerQuad);

            break;
        }

        default: {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSession::endFrame(). Unsupported layer type. Ignoring layer.");
        }
        }
    }

    // Submit the frame
    XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
    frameEndInfo.displayTime = options.displayTime;
    frameEndInfo.environmentBlendMode = environmentBlendModeToXrEnvironmentBlendMode(options.environmentBlendMode);
    frameEndInfo.layerCount = static_cast<uint32_t>(layerCount);
    frameEndInfo.layers = xrLayers.data();
    const auto result = xrEndFrame(session, &frameEndInfo);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to end frame.");
    }
    return static_cast<EndFrameResult>(result);
}

LocateViewsResult OpenXrSession::locateViews(const LocateViewsOptions &options, ViewConfigurationType viewConfigurationType, ViewState &viewState)
{
    OpenXrReferenceSpace *openxrReferenceSpace = openxrResourceManager->getReferenceSpace(options.referenceSpace);
    assert(openxrReferenceSpace);

    // Ensure our local storage is large enough to hold the number of views in the session
    const auto requiredViewCount = viewCount(viewConfigurationType);
    if (xrViews.size() < requiredViewCount) {
        xrViews.resize(requiredViewCount, { XR_TYPE_VIEW });
    }

    // Locate the views relative to the reference space
    XrViewState xrViewState{ XR_TYPE_VIEW_STATE }; // Contains information on whether the position and/or orientation is valid and/or tracked.
    XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
    viewLocateInfo.viewConfigurationType = viewConfigurationTypeToXrViewConfigurationType(viewConfigurationType);
    viewLocateInfo.displayTime = options.displayTime;
    viewLocateInfo.space = openxrReferenceSpace->referenceSpace;
    uint32_t viewCount = 0;
    const auto result = xrLocateViews(session, &viewLocateInfo, &xrViewState, requiredViewCount, &viewCount, xrViews.data());
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to locate views.");
        return static_cast<LocateViewsResult>(result);
    }

    // Ensure the viewState views container is large enough
    if (viewState.views.size() < viewCount)
        return LocateViewsResult::SizeInsufficient;

    // Update the view state
    viewState.viewStateFlags = xrViewStateFlagsToViewStateFlags(xrViewState.viewStateFlags);
    viewState.viewCount = viewCount;
    for (uint32_t i = 0; i < viewCount; ++i) {
        auto &view = viewState.views[i];
        const auto &xrView = xrViews[i];

        view.pose = xrPoseToPose(xrView.pose);
        view.fieldOfView = xrFovToFov(xrView.fov);
    }

    return LocateViewsResult::Success;
}

void OpenXrSession::setSessionState(SessionState state)
{
    // Forward on fine-grained state to frontend session
    SPDLOG_LOGGER_INFO(Logger::logger(), "OpenXrSession::setSessionState() state: {}", state);
    frontendSession->state = state;

    if (frontendSession->autoRun() != true)
        return;

    if (state == SessionState::Ready) {
        XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
        sessionBeginInfo.primaryViewConfigurationType = viewConfigurationTypeToXrViewConfigurationType(frontendSession->viewConfigurationType());
        if (xrBeginSession(session, &sessionBeginInfo) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to begin session.");
            return;
        }

        frontendSession->running = true;
    } else if (state == SessionState::Stopping) {
        if (xrEndSession(session) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to end session.");
            return;
        }

        frontendSession->running = false;
    }
}

} // namespace KDXr
