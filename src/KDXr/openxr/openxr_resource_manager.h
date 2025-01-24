/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/resource_manager.h>
#include <KDXr/openxr/openxr_action.h>
#include <KDXr/openxr/openxr_action_set.h>
#include <KDXr/openxr/openxr_instance.h>
#include <KDXr/openxr/openxr_reference_space.h>
#include <KDXr/openxr/openxr_session.h>
#include <KDXr/openxr/openxr_swapchain.h>
#include <KDXr/openxr/openxr_system.h>
#include <KDXr/openxr/openxr_passthrough_layer.h>

#include <KDXr/kdxr_export.h>

#include <KDGpu/pool.h>

#include <openxr/openxr.h>

namespace KDXr {

/**
 * @brief OpenXrResourceManager
 * \ingroup openxr
 *
 */

class KDXR_EXPORT OpenXrResourceManager final : public ResourceManager
{
public:
    OpenXrResourceManager();
    ~OpenXrResourceManager() final;

    std::vector<ApiLayer> availableApiLayers() const final;
    std::vector<Extension> availableInstanceExtensions() const final;

    KDGpu::Handle<Instance_t> createInstance(const InstanceOptions &options) final;
    void deleteInstance(const KDGpu::Handle<Instance_t> &handle) final;
    OpenXrInstance *getInstance(const KDGpu::Handle<Instance_t> &handle) const final;

    KDGpu::Handle<System_t> insertSystem(const OpenXrSystem &openXrSystem);
    void removeSystem(const KDGpu::Handle<System_t> &handle) final;
    OpenXrSystem *getSystem(const KDGpu::Handle<System_t> &handle) const final;

    KDGpu::Handle<Session_t> createSession(const KDGpu::Handle<System_t> &systemHandle, const SessionOptions &options) final;
    void deleteSession(const KDGpu::Handle<Session_t> &handle) final;
    OpenXrSession *getSession(const KDGpu::Handle<Session_t> &handle) const final;

    KDGpu::Handle<ReferenceSpace_t> createReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, const ReferenceSpaceOptions &options) final;
    KDGpu::Handle<ReferenceSpace_t> createReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, const ActionSpaceOptions &options) final;
    void deleteReferenceSpace(const KDGpu::Handle<ReferenceSpace_t> &handle) final;
    OpenXrReferenceSpace *getReferenceSpace(const KDGpu::Handle<ReferenceSpace_t> &handle) const final;

    KDGpu::Handle<PassthroughLayer_t> createPassthroughLayer(const KDGpu::Handle<Session_t> &sessionHandle, const PassthroughLayerOptions &options) final;
    void deletePassthroughLayer(const KDGpu::Handle<PassthroughLayer_t> &handle) final;
    OpenXrPassthroughLayer *getPassthroughLayer(const KDGpu::Handle<PassthroughLayer_t> &handle) const;

    KDGpu::Handle<Swapchain_t> createSwapchain(const KDGpu::Handle<Session_t> &sessionHandle, const SwapchainOptions &options) final;
    void deleteSwapchain(const KDGpu::Handle<Swapchain_t> &handle) final;
    OpenXrSwapchain *getSwapchain(const KDGpu::Handle<Swapchain_t> &handle) const final;

    KDGpu::Handle<ActionSet_t> createActionSet(const KDGpu::Handle<Instance_t> &instanceHandle, const ActionSetOptions &options) final;
    void deleteActionSet(const KDGpu::Handle<ActionSet_t> &handle) final;
    OpenXrActionSet *getActionSet(const KDGpu::Handle<ActionSet_t> &handle) const final;

    KDGpu::Handle<Action_t> createAction(const KDGpu::Handle<ActionSet_t> &actionSetHandle, const ActionOptions &options) final;
    void deleteAction(const KDGpu::Handle<Action_t> &handle) final;
    OpenXrAction *getAction(const KDGpu::Handle<Action_t> &handle) const final;

private:
    KDGpu::Pool<OpenXrInstance, Instance_t> m_instances{ 1 };
    KDGpu::Pool<OpenXrSystem, System_t> m_systems{ 1 };
    KDGpu::Pool<OpenXrSession, Session_t> m_sessions{ 1 };
    KDGpu::Pool<OpenXrReferenceSpace, ReferenceSpace_t> m_referenceSpaces{ 4 };
    KDGpu::Pool<OpenXrSwapchain, Swapchain_t> m_swapchains{ 4 };
    KDGpu::Pool<OpenXrActionSet, ActionSet_t> m_actionSets{ 4 };
    KDGpu::Pool<OpenXrAction, Action_t> m_actions{ 32 };
    KDGpu::Pool<OpenXrPassthroughLayer, PassthroughLayer_t> m_passthroughLayers{ 1 };

    XrPassthroughFB m_passthrough{ XR_NULL_HANDLE };
};

} // namespace KDXr
