/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_session.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

#include <vector>

namespace KDGpu {
class GraphicsApi;
struct Device_t;
} // namespace KDGpu

namespace KDXr {

class OpenXrResourceManager;
struct Instance_t;
struct System_t;

/**
 * @brief OpenXrSession
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSession : public ApiSession {
    explicit OpenXrSession(OpenXrResourceManager *_openxrResourceManager,
                           XrSession _session,
                           const Handle<System_t> _systemHandle,
                           const Handle<Instance_t> _instanceHandle,
                           KDGpu::GraphicsApi *_graphicsApi,
                           KDGpu::Handle<KDGpu::Device_t> _device,
                           uint32_t queueIndex) noexcept;

    void initialize(Session *_frontendSession) final;
    std::vector<KDGpu::Format> supportedSwapchainFormats() const final;
    FrameState waitForFrame() final;
    BeginFrameResult beginFrame() final;
    EndFrameResult endFrame(const EndFrameOptions &options) final;

    LocateViewsResult locateViews(const LocateViewsOptions &options, ViewConfigurationType viewConfigurationType, ViewState &viewState) final;

    void setSessionState(SessionState state);

    AttachActionSetsResult attachActionSets(const AttachActionSetsOptions &options) final;
    InteractionProfileState getInteractionProfile(const GetInterationProfileOptions &options) const final;
    SyncActionsResult syncActions(const SyncActionsOptions &options) final;
    GetActionStateResult getBooleanState(const GetActionStateOptions &options, ActionStateBoolean &state) const final;
    GetActionStateResult getFloatState(const GetActionStateOptions &options, ActionStateFloat &state) const final;
    VibrateOutputResult vibrateOutput(const VibrationOutputOptions &options) final;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSession session{ XR_NULL_HANDLE };
    Handle<System_t> systemHandle;
    Handle<Instance_t> instanceHandle;

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
    std::vector<XrCompositionLayerQuad> xrLayerQuads;
    std::vector<XrCompositionLayerCylinderKHR> xrLayerCylinders;
};

} // namespace KDXr
