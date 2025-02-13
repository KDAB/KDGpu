/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>
#include <KDXr/compositor.h>
#include <KDXr/locate_views_options.h>

#include <KDGpu/handle.h>
#include <KDGpu/graphics_api.h>
#include <KDGpu/gpu_core.h>

#include <openxr/openxr.h>

#include <vector>

namespace KDGpu {
struct Device_t;
} // namespace KDGpu

namespace KDXr {

class Session;
class OpenXrResourceManager;
struct Instance_t;
struct System_t;
struct AttachActionSetsOptions;
struct GetActionStateOptions;
struct GetInterationProfileOptions;
struct Session_t;
struct SyncActionsOptions;
struct VibrationOutputOptions;

/**
 * @brief OpenXrSession
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSession {
    explicit OpenXrSession(OpenXrResourceManager *_openxrResourceManager,
                           XrSession _session,
                           const KDGpu::Handle<System_t> _systemHandle,
                           const KDGpu::Handle<Instance_t> _instanceHandle,
                           KDGpu::GraphicsApi *_graphicsApi,
                           KDGpu::Handle<KDGpu::Device_t> _device,
                           uint32_t queueIndex) noexcept;

    void initialize(Session *_frontendSession);
    std::vector<KDGpu::Format> supportedSwapchainFormats() const;
    FrameState waitForFrame();
    BeginFrameResult beginFrame();
    EndFrameResult endFrame(const EndFrameOptions &options);

    LocateViewsResult locateViews(const LocateViewsOptions &options, ViewConfigurationType viewConfigurationType, ViewState &viewState);

    void setSessionState(SessionState state);

    AttachActionSetsResult attachActionSets(const AttachActionSetsOptions &options);
    InteractionProfileState getInteractionProfile(const GetInterationProfileOptions &options) const;
    SyncActionsResult syncActions(const SyncActionsOptions &options);
    GetActionStateResult getBooleanState(const GetActionStateOptions &options, ActionStateBoolean &state) const;
    GetActionStateResult getFloatState(const GetActionStateOptions &options, ActionStateFloat &state) const;
    GetActionStateResult getVector2State(const GetActionStateOptions &options, ActionStateVector2 &state) const;
    GetActionStateResult getPoseState(const GetActionStateOptions &options, ActionStatePose &state) const;
    VibrateOutputResult vibrateOutput(const VibrationOutputOptions &options);

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSession session{ XR_NULL_HANDLE };
    KDGpu::Handle<System_t> systemHandle;
    KDGpu::Handle<Instance_t> instanceHandle;
    bool supportsCompositorLayerDepth{ false };

    // Graphics related stuff
    KDGpu::GraphicsApi *graphicsApi{ nullptr };
    KDGpu::Handle<KDGpu::Device_t> deviceHandle;
    uint32_t queueIndex{ 0 };

    Session *frontendSession{ nullptr };

    // Local storage to avoid using a temporary vector every frame and the allocations that entails.
    // This will maintain a high-water mark of the number of views in the session. Initialised to 2
    // as that is the most common number of views.
    std::vector<XrView> xrViews{ 2, { XR_TYPE_VIEW } };

    // Composition layers
    // Local storage to avoid using a temporary vectors every frame and the allocations that entails.
    std::vector<XrCompositionLayerBaseHeader *> xrLayers;
    std::vector<XrCompositionLayerProjection> xrLayerProjections;
    std::vector<XrCompositionLayerProjectionView> xrLayerProjectionViews;
    std::vector<XrCompositionLayerDepthInfoKHR> xrLayerDepthInfos;
    std::vector<XrCompositionLayerDepthTestFB> xrLayerDepthTests;
    std::vector<XrCompositionLayerQuad> xrLayerQuads;
    std::vector<XrCompositionLayerCylinderKHR> xrLayerCylinders;
    std::vector<XrCompositionLayerCubeKHR> xrLayerCubes;
    std::vector<XrCompositionLayerPassthroughFB> xrLayerPassthrough;
};

} // namespace KDXr
