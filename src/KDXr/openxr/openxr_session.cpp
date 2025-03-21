/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_session.h"

#include <KDXr/session.h>
#include <KDXr/openxr/openxr_enums.h>
#include <KDXr/openxr/openxr_instance.h>
#include <KDXr/openxr/openxr_resource_manager.h>
#include <KDXr/utils/formatters.h>
#include <KDXr/utils/logging.h>

#include <KDGpu/api/graphics_api_impl.h>

#include <algorithm>

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
                             const KDGpu::Handle<System_t> _systemHandle,
                             const KDGpu::Handle<Instance_t> _instanceHandle,
                             KDGpu::GraphicsApi *_graphicsApi,
                             KDGpu::Handle<KDGpu::Device_t> _device,
                             uint32_t queueIndex) noexcept
    : openxrResourceManager(_openxrResourceManager)
    , session(_session)
    , systemHandle(_systemHandle)
    , instanceHandle(_instanceHandle)
    , graphicsApi(_graphicsApi)
    , deviceHandle(_device)
    , queueIndex(queueIndex)
{
    // If the instance extensions includes XR_FB_composition_layer_depth_test, mark it as supported
    // so we can query it efficiently in OpenXRSession::endFrame().
    OpenXrInstance *openXrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openXrInstance);
    supportsCompositorLayerDepth = std::any_of(openXrInstance->extensions.begin(), openXrInstance->extensions.end(), [](const Extension &extension) {
        return extension.name == XR_FB_COMPOSITION_LAYER_DEPTH_TEST_EXTENSION_NAME;
    });
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
    if (graphicsApi->api() == KDGpu::ApiType::Vulkan) {
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
    if (supportsCompositorLayerDepth)
        xrLayerDepthTests.reserve(layerCount);

    // Clear the various types of projection layer containers. The first time we call endFrame(),
    // the vectors will begin to grow to the size of the number of layers. After that, they will
    // maintain a high-water mark of the number of layers of each type.
    xrLayerProjections.clear();
    xrLayerProjectionViews.clear();
    xrLayerDepthInfos.clear();
    xrLayerDepthTests.clear();
    xrLayerCylinders.clear();
    xrLayerQuads.clear();
    xrLayerCubes.clear();
    xrLayerPassthrough.clear();

    for (size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        switch (options.layers[layerIndex]->type) {
        case CompositionLayerType::Projection: {
            auto &projectionLayer = reinterpret_cast<ProjectionLayer &>(*options.layers[layerIndex]);
            const auto viewCount = projectionLayer.views.size();

            // Reserve space for the views and depth infos. We do this up front so that we do not invalidate the pointers
            // to the views and depth infos when we add the next pointers to the views.
            xrLayerProjectionViews.reserve(viewCount);
            xrLayerDepthInfos.reserve(viewCount);

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

                auto openxrDepthSwapchain = openxrResourceManager->getSwapchain(projectionLayer.depthInfos[viewIndex].depthSwapchainSubTexture.swapchain);
                assert(openxrDepthSwapchain);

                xrLayerDepthInfos.push_back({ XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR });
                auto &depthInfo = xrLayerDepthInfos.back();
                depthInfo.subImage.swapchain = openxrDepthSwapchain->swapchain;
                depthInfo.subImage.imageRect = rect2DToXrRecti(projectionLayer.depthInfos[viewIndex].depthSwapchainSubTexture.rect);
                depthInfo.subImage.imageArrayIndex = projectionLayer.depthInfos[viewIndex].depthSwapchainSubTexture.arrayIndex;
                depthInfo.minDepth = projectionLayer.depthInfos[viewIndex].minDepth;
                depthInfo.maxDepth = projectionLayer.depthInfos[viewIndex].maxDepth;
                depthInfo.nearZ = projectionLayer.depthInfos[viewIndex].nearZ;
                depthInfo.farZ = projectionLayer.depthInfos[viewIndex].farZ;

                projectionView.next = reinterpret_cast<XrCompositionLayerBaseHeader *>(&depthInfo);
            }

            auto openxrReferenceSpace = openxrResourceManager->getReferenceSpace(projectionLayer.referenceSpace);
            assert(openxrReferenceSpace);

            xrLayerProjections.push_back({ XR_TYPE_COMPOSITION_LAYER_PROJECTION });
            auto &projectionLayerProjection = xrLayerProjections.back();
            projectionLayerProjection.layerFlags = compositionLayerFlagsToXrCompositionLayerFlags(projectionLayer.flags);
            projectionLayerProjection.space = openxrReferenceSpace->referenceSpace;
            projectionLayerProjection.viewCount = static_cast<uint32_t>(viewCount);
            projectionLayerProjection.views = xrLayerProjectionViews.data() + xrLayerProjectionViews.size() - viewCount;

            if (supportsCompositorLayerDepth) {
                xrLayerDepthTests.push_back({ XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB });
                auto &layerDepthTest = xrLayerDepthTests.back();
                layerDepthTest.depthMask = XR_TRUE;
                layerDepthTest.compareOp = XR_COMPARE_OP_LESS_FB;
                projectionLayerProjection.next = &layerDepthTest;
            }

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

            if (supportsCompositorLayerDepth) {
                xrLayerDepthTests.push_back({ XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB });
                auto &layerDepthTest = xrLayerDepthTests.back();
                layerDepthTest.depthMask = XR_TRUE;
                layerDepthTest.compareOp = XR_COMPARE_OP_LESS_FB;
                quadLayerQuad.next = &layerDepthTest;
            }

            xrLayers[layerIndex] = reinterpret_cast<XrCompositionLayerBaseHeader *>(&quadLayerQuad);

            break;
        }

        case CompositionLayerType::Cylinder: {
            auto &cylinderLayer = reinterpret_cast<CylinderLayer &>(*options.layers[layerIndex]);

            auto openxrSwapchain = openxrResourceManager->getSwapchain(cylinderLayer.swapchainSubTexture.swapchain);
            assert(openxrSwapchain);
            auto openxrReferenceSpace = openxrResourceManager->getReferenceSpace(cylinderLayer.referenceSpace);
            assert(openxrReferenceSpace);

            xrLayerCylinders.push_back({ XR_TYPE_COMPOSITION_LAYER_CYLINDER_KHR });
            auto &cylinderLayerCylinder = xrLayerCylinders.back();
            cylinderLayerCylinder.layerFlags = compositionLayerFlagsToXrCompositionLayerFlags(cylinderLayer.flags);
            cylinderLayerCylinder.space = openxrReferenceSpace->referenceSpace;
            cylinderLayerCylinder.eyeVisibility = eyeVisibilityToXrEyeVisibility(cylinderLayer.eyeVisibility);
            cylinderLayerCylinder.subImage.swapchain = openxrSwapchain->swapchain;
            cylinderLayerCylinder.subImage.imageRect = rect2DToXrRecti(cylinderLayer.swapchainSubTexture.rect);
            cylinderLayerCylinder.subImage.imageArrayIndex = cylinderLayer.swapchainSubTexture.arrayIndex;
            cylinderLayerCylinder.pose = poseToXrPose(cylinderLayer.pose);
            cylinderLayerCylinder.radius = cylinderLayer.radius;
            cylinderLayerCylinder.centralAngle = cylinderLayer.centralAngle;
            cylinderLayerCylinder.aspectRatio = cylinderLayer.aspectRatio;

            if (supportsCompositorLayerDepth) {
                xrLayerDepthTests.push_back({ XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB });
                auto &layerDepthTest = xrLayerDepthTests.back();
                layerDepthTest.depthMask = XR_TRUE;
                layerDepthTest.compareOp = XR_COMPARE_OP_LESS_FB;
                cylinderLayerCylinder.next = &layerDepthTest;
            }

            xrLayers[layerIndex] = reinterpret_cast<XrCompositionLayerBaseHeader *>(&cylinderLayerCylinder);

            break;
        }

        case CompositionLayerType::Cube: {
            auto &cubeLayer = reinterpret_cast<CubeLayer &>(*options.layers[layerIndex]);

            auto openxrSwapchain = openxrResourceManager->getSwapchain(cubeLayer.swapchain);
            assert(openxrSwapchain);
            auto openxrReferenceSpace = openxrResourceManager->getReferenceSpace(cubeLayer.referenceSpace);
            assert(openxrReferenceSpace);

            xrLayerCubes.push_back({ XR_TYPE_COMPOSITION_LAYER_CUBE_KHR });
            auto &cubeLayerCube = xrLayerCubes.back();
            cubeLayerCube.layerFlags = compositionLayerFlagsToXrCompositionLayerFlags(cubeLayer.flags);
            cubeLayerCube.space = openxrReferenceSpace->referenceSpace;
            cubeLayerCube.eyeVisibility = eyeVisibilityToXrEyeVisibility(cubeLayer.eyeVisibility);
            cubeLayerCube.swapchain = openxrSwapchain->swapchain;
            cubeLayerCube.imageArrayIndex = cubeLayer.arrayIndex;
            cubeLayerCube.orientation = XrQuaternionf{
                cubeLayer.orientation.x,
                cubeLayer.orientation.y,
                cubeLayer.orientation.z,
                cubeLayer.orientation.w
            };

            if (supportsCompositorLayerDepth) {
                xrLayerDepthTests.push_back({ XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB });
                auto &layerDepthTest = xrLayerDepthTests.back();
                layerDepthTest.depthMask = XR_TRUE;
                layerDepthTest.compareOp = XR_COMPARE_OP_LESS_FB;
                cubeLayerCube.next = &layerDepthTest;
            }

            xrLayers[layerIndex] = reinterpret_cast<XrCompositionLayerBaseHeader *>(&cubeLayerCube);

            break;
        }

        case CompositionLayerType::PassThrough: {
            auto &passthroughLayer = reinterpret_cast<PassthroughCompositionLayer &>(*options.layers[layerIndex]);

            xrLayerPassthrough.push_back({ XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB });
            auto &passthroughLayerPassthrough = xrLayerPassthrough.back();
            passthroughLayerPassthrough.flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
            passthroughLayerPassthrough.space = XR_NULL_HANDLE;

            auto openxrPassthroughLayer = openxrResourceManager->getPassthroughLayer(passthroughLayer.passthroughLayer);
            assert(openxrPassthroughLayer);
            passthroughLayerPassthrough.layerHandle = openxrPassthroughLayer->passthroughLayer;

            if (supportsCompositorLayerDepth) {
                xrLayerDepthTests.push_back({ XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB });
                auto &layerDepthTest = xrLayerDepthTests.back();
                layerDepthTest.depthMask = XR_TRUE;
                layerDepthTest.compareOp = XR_COMPARE_OP_ALWAYS_FB;
                passthroughLayerPassthrough.next = &layerDepthTest;
            }

            xrLayers[layerIndex] = reinterpret_cast<XrCompositionLayerBaseHeader *>(&passthroughLayerPassthrough);

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
        viewState.views.resize(viewCount);

    // Update the view state
    viewState.viewStateFlags = xrViewStateFlagsToViewStateFlags(xrViewState.viewStateFlags);
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

AttachActionSetsResult OpenXrSession::attachActionSets(const AttachActionSetsOptions &options)
{
    std::vector<XrActionSet> actionSets;
    actionSets.reserve(options.actionSets.size());
    for (const auto &actionSet : options.actionSets) {
        auto openxrActionSet = openxrResourceManager->getActionSet(actionSet);
        assert(openxrActionSet);
        actionSets.push_back(openxrActionSet->actionSet);
    }

    XrSessionActionSetsAttachInfo sessionActionSetsAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    sessionActionSetsAttachInfo.countActionSets = static_cast<uint32_t>(actionSets.size());
    sessionActionSetsAttachInfo.actionSets = actionSets.data();

    const auto result = xrAttachSessionActionSets(session, &sessionActionSetsAttachInfo);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to attach action sets.");
    }
    return static_cast<AttachActionSetsResult>(result);
}

InteractionProfileState OpenXrSession::getInteractionProfile(const GetInterationProfileOptions &options) const
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);
    XrPath xrPath = openxrInstance->createXrPath(options.topLevelUserPath);
    XrInteractionProfileState xrInteractionProfileState{ XR_TYPE_INTERACTION_PROFILE_STATE };
    const auto result = xrGetCurrentInteractionProfile(session, xrPath, &xrInteractionProfileState);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get interaction profile.");
        return InteractionProfileState{ .result = static_cast<GetInteractionProfileResult>(result) };
    }
    return InteractionProfileState{
        .result = static_cast<GetInteractionProfileResult>(result),
        .interactionProfile = openxrInstance->pathToString(xrInteractionProfileState.interactionProfile)
    };
}

SyncActionsResult OpenXrSession::syncActions(const SyncActionsOptions &options)
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);

    std::vector<XrActiveActionSet> activeActionSets;
    activeActionSets.reserve(options.actionSets.size());
    for (const auto &actionSet : options.actionSets) {
        auto openxrActionSet = openxrResourceManager->getActionSet(actionSet.actionSet);
        assert(openxrActionSet);
        XrPath xrPath{ XR_NULL_PATH };
        if (!actionSet.subactionPath.empty())
            xrPath = openxrInstance->createXrPath(actionSet.subactionPath);
        activeActionSets.push_back({ openxrActionSet->actionSet, xrPath });
    }

    XrActionsSyncInfo actionsSyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
    actionsSyncInfo.countActiveActionSets = static_cast<uint32_t>(activeActionSets.size());
    actionsSyncInfo.activeActionSets = activeActionSets.data();

    const auto result = xrSyncActions(session, &actionsSyncInfo);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to sync actions.");
    }
    return static_cast<SyncActionsResult>(result);
}

GetActionStateResult OpenXrSession::getBooleanState(const GetActionStateOptions &options, ActionStateBoolean &state) const
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);
    XrPath xrPath{ XR_NULL_PATH };
    if (!options.subactionPath.empty())
        xrPath = openxrInstance->createXrPath(options.subactionPath);

    OpenXrAction *openxrAction = openxrResourceManager->getAction(options.action);
    assert(openxrAction);

    XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    actionStateGetInfo.action = openxrAction->action;
    actionStateGetInfo.subactionPath = xrPath;
    XrActionStateBoolean xrActionStateBoolean{ XR_TYPE_ACTION_STATE_BOOLEAN };
    const auto result = xrGetActionStateBoolean(session, &actionStateGetInfo, &xrActionStateBoolean);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get action state.");
        state = ActionStateBoolean{};
    } else {
        state = ActionStateBoolean{
            .currentState = static_cast<bool>(xrActionStateBoolean.currentState),
            .changedSinceLastSync = static_cast<bool>(xrActionStateBoolean.changedSinceLastSync),
            .lastChangeTime = xrActionStateBoolean.lastChangeTime,
            .active = static_cast<bool>(xrActionStateBoolean.isActive)
        };
    }

    return static_cast<GetActionStateResult>(result);
}

GetActionStateResult OpenXrSession::getFloatState(const GetActionStateOptions &options, ActionStateFloat &state) const
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);
    XrPath xrPath{ XR_NULL_PATH };
    if (!options.subactionPath.empty())
        xrPath = openxrInstance->createXrPath(options.subactionPath);

    OpenXrAction *openxrAction = openxrResourceManager->getAction(options.action);
    assert(openxrAction);

    XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    actionStateGetInfo.action = openxrAction->action;
    actionStateGetInfo.subactionPath = xrPath;
    XrActionStateFloat xrActionStateFloat{ XR_TYPE_ACTION_STATE_FLOAT };
    const auto result = xrGetActionStateFloat(session, &actionStateGetInfo, &xrActionStateFloat);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get action state.");
        state = ActionStateFloat{};
    } else {
        state = ActionStateFloat{
            .currentState = xrActionStateFloat.currentState,
            .changedSinceLastSync = static_cast<bool>(xrActionStateFloat.changedSinceLastSync),
            .lastChangeTime = xrActionStateFloat.lastChangeTime,
            .active = static_cast<bool>(xrActionStateFloat.isActive)
        };
    }

    return static_cast<GetActionStateResult>(result);
}

GetActionStateResult OpenXrSession::getVector2State(const GetActionStateOptions &options, ActionStateVector2 &state) const
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);
    XrPath xrPath{ XR_NULL_PATH };
    if (!options.subactionPath.empty())
        xrPath = openxrInstance->createXrPath(options.subactionPath);

    OpenXrAction *openxrAction = openxrResourceManager->getAction(options.action);
    assert(openxrAction);

    XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    actionStateGetInfo.action = openxrAction->action;
    actionStateGetInfo.subactionPath = xrPath;
    XrActionStateVector2f xrActionStateVector2f{ XR_TYPE_ACTION_STATE_VECTOR2F };
    const auto result = xrGetActionStateVector2f(session, &actionStateGetInfo, &xrActionStateVector2f);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get action state.");
        state = ActionStateVector2{};
    } else {
        state = ActionStateVector2{
            .currentState = KDXr::Vector2{ xrActionStateVector2f.currentState.x, xrActionStateVector2f.currentState.y },
            .changedSinceLastSync = static_cast<bool>(xrActionStateVector2f.changedSinceLastSync),
            .lastChangeTime = xrActionStateVector2f.lastChangeTime,
            .active = static_cast<bool>(xrActionStateVector2f.isActive)
        };
    }

    return static_cast<GetActionStateResult>(result);
}

GetActionStateResult OpenXrSession::getPoseState(const GetActionStateOptions &options, ActionStatePose &state) const
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);
    XrPath xrPath{ XR_NULL_PATH };
    if (!options.subactionPath.empty())
        xrPath = openxrInstance->createXrPath(options.subactionPath);

    OpenXrAction *openxrAction = openxrResourceManager->getAction(options.action);
    assert(openxrAction);

    XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
    actionStateGetInfo.action = openxrAction->action;
    actionStateGetInfo.subactionPath = xrPath;
    XrActionStatePose xrActionStatePose{ XR_TYPE_ACTION_STATE_POSE };
    const auto result = xrGetActionStatePose(session, &actionStateGetInfo, &xrActionStatePose);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get action state.");
        state = ActionStatePose{};
    } else {
        state = ActionStatePose{ .active = static_cast<bool>(xrActionStatePose.isActive) };
    }

    return static_cast<GetActionStateResult>(result);
}

VibrateOutputResult OpenXrSession::vibrateOutput(const VibrationOutputOptions &options)
{
    OpenXrInstance *openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    assert(openxrInstance);
    XrPath xrPath{ XR_NULL_PATH };
    if (!options.subactionPath.empty())
        xrPath = openxrInstance->createXrPath(options.subactionPath);

    OpenXrAction *openxrAction = openxrResourceManager->getAction(options.action);
    assert(openxrAction);

    XrHapticVibration vibration{ XR_TYPE_HAPTIC_VIBRATION };
    vibration.amplitude = options.amplitude;
    vibration.duration = options.duration;
    vibration.frequency = options.frequency;

    XrHapticActionInfo hapticActionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
    hapticActionInfo.action = openxrAction->action;
    hapticActionInfo.subactionPath = xrPath;
    const auto result = xrApplyHapticFeedback(session, &hapticActionInfo, reinterpret_cast<const XrHapticBaseHeader *>(&vibration));
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to apply haptic feedback.");
    }
    return static_cast<VibrateOutputResult>(result);
}

} // namespace KDXr
