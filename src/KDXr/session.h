/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/compositor.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/locate_views_options.h>
#include <KDXr/passthrough_layer_controller.h>
#include <KDXr/reference_space.h>
#include <KDXr/swapchain.h>

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <kdbindings/property.h>

#include <span>
#include <vector>

namespace KDGpu {
struct Device_t;
struct Queue_t;
} // namespace KDGpu

namespace KDXr {

struct Action_t;
struct ActionSet_t;
struct Session_t;
struct System_t;
class System;
class XrApi;

/**
    @brief Holds option fields used for Session creation
    @ingroup public
    @headerfile session.h <KDXr/session.h>
 */
struct SessionOptions {
    KDGpu::GraphicsApi *graphicsApi{ nullptr };
    KDGpu::Handle<KDGpu::Device_t> device;
    uint32_t queueIndex{ 0 };
};

struct AttachActionSetsOptions {
    std::vector<KDGpu::Handle<ActionSet_t>> actionSets;
};

struct ActiveActionSet {
    KDGpu::Handle<ActionSet_t> actionSet;
    std::string subactionPath;
};

struct GetInterationProfileOptions {
    std::string topLevelUserPath;
};

struct SyncActionsOptions {
    std::vector<ActiveActionSet> actionSets;
};

struct GetActionStateOptions {
    KDGpu::Handle<Action_t> action;
    std::string subactionPath;
};

// TODO: Is this enough? Do we need any other types of output?
struct VibrationOutputOptions {
    KDGpu::Handle<Action_t> action;
    std::string subactionPath;
    Duration duration{ MinimumHapticDuration };
    float amplitude{ 0.0f };
    float frequency{ UnspecifiedHapticFrequency };
};

class KDXR_EXPORT Session
{
public:
    KDBindings::Property<SessionState> state{ SessionState::Unknown };
    KDBindings::Property<bool> running{ false };
    KDBindings::Property<bool> autoRun{ true };

    Session();
    ~Session();

    Session(Session &&);
    Session &operator=(Session &&);

    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;

    KDGpu::Handle<Session_t> handle() const noexcept { return m_session; }
    bool isValid() const { return m_session.isValid(); }

    operator KDGpu::Handle<Session_t>() const noexcept { return m_session; }

    ReferenceSpace createReferenceSpace(const ReferenceSpaceOptions &options = ReferenceSpaceOptions());

    PassthroughLayerController createPassthroughLayer(const PassthroughLayerOptions &options = PassthroughLayerOptions());
    void setPassthroughRunning(KDGpu::Handle<System_t>, bool running);

    std::span<const KDGpu::Format>
    supportedSwapchainFormats() const;
    KDGpu::Format selectSwapchainFormat(std::span<const KDGpu::Format> preferredFormats) const;

    Swapchain createSwapchain(const SwapchainOptions &options);

    void setViewConfigurationType(ViewConfigurationType viewConfigurationType) { m_viewConfigurationType = viewConfigurationType; }
    ViewConfigurationType viewConfigurationType() const { return m_viewConfigurationType; }

    bool isActive() const { return state() == SessionState::Synchronized || state() == SessionState::Focused || state() == SessionState::Visible; }

    FrameState waitForFrame();
    BeginFrameResult beginFrame();
    EndFrameResult endFrame(const EndFrameOptions &options);

    LocateViewsResult locateViews(const LocateViewsOptions &options, ViewState &viewState);

    AttachActionSetsResult attachActionSets(const AttachActionSetsOptions &options);
    InteractionProfileState getInteractionProfile(const GetInterationProfileOptions &options) const;
    SyncActionsResult syncActions(const SyncActionsOptions &options);

    ReferenceSpace createActionSpace(const ActionSpaceOptions &options);

    // TODO: Should we add per type getters? Or perhaps have type-specific actions and move getState to the actions?
    // If we do that, then we need the backend actions to be able to know about the session.
    GetActionStateResult getBooleanState(const GetActionStateOptions &options, ActionStateBoolean &state) const;
    GetActionStateResult getFloatState(const GetActionStateOptions &options, ActionStateFloat &state) const;
    GetActionStateResult getVector2State(const GetActionStateOptions &options, ActionStateVector2 &state) const;
    GetActionStateResult getPoseState(const GetActionStateOptions &options, ActionStatePose &state) const;
    VibrateOutputResult vibrateOutput(const VibrationOutputOptions &options);

private:
    Session(const KDGpu::Handle<System_t> &systemHandle, XrApi *api, const SessionOptions &options);

    XrApi *m_api{ nullptr };
    KDGpu::Handle<System_t> m_systemHandle;
    KDGpu::Handle<Session_t> m_session;

    mutable std::vector<KDGpu::Format> m_supportedSwapchainFormats;
    ViewConfigurationType m_viewConfigurationType{ ViewConfigurationType::PrimaryStereo };

    friend class System;
};

} // namespace KDXr
