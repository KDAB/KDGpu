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
#include <KDXr/instance.h>

#include <KDGpu/pool.h>

#include <openxr/openxr.h>

namespace KDXr {

/**
 * @brief OpenXrResourceManager
 * \ingroup openxr
 *
 */

class KDXR_EXPORT OpenXrResourceManager
{
public:
    OpenXrResourceManager();
    ~OpenXrResourceManager();

    std::vector<ApiLayer> availableApiLayers() const;
    std::vector<Extension> availableInstanceExtensions() const;

    KDGpu::Handle<Instance_t> createInstance(const InstanceOptions &options);
    void deleteInstance(const KDGpu::Handle<Instance_t> &handle);
    OpenXrInstance *getInstance(const KDGpu::Handle<Instance_t> &handle) const;

    KDGpu::Handle<System_t> insertSystem(const OpenXrSystem &openXrSystem);
    void removeSystem(const KDGpu::Handle<System_t> &handle);
    OpenXrSystem *getSystem(const KDGpu::Handle<System_t> &handle) const;

    KDGpu::Handle<Session_t> createSession(const KDGpu::Handle<System_t> &systemHandle, const SessionOptions &options);
    void deleteSession(const KDGpu::Handle<Session_t> &handle);
    OpenXrSession *getSession(const KDGpu::Handle<Session_t> &handle) const;

    KDGpu::Handle<ReferenceSpace_t> createReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, const ReferenceSpaceOptions &options);
    KDGpu::Handle<ReferenceSpace_t> createReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, const ActionSpaceOptions &options);
    void deleteReferenceSpace(const KDGpu::Handle<ReferenceSpace_t> &handle);
    OpenXrReferenceSpace *getReferenceSpace(const KDGpu::Handle<ReferenceSpace_t> &handle) const;

    KDGpu::Handle<PassthroughLayer_t> createPassthroughLayer(const KDGpu::Handle<Session_t> &sessionHandle, const PassthroughLayerOptions &options);
    void deletePassthroughLayer(const KDGpu::Handle<PassthroughLayer_t> &handle);
    OpenXrPassthroughLayer *getPassthroughLayer(const KDGpu::Handle<PassthroughLayer_t> &handle) const;

    KDGpu::Handle<Swapchain_t> createSwapchain(const KDGpu::Handle<Session_t> &sessionHandle, const SwapchainOptions &options);
    void deleteSwapchain(const KDGpu::Handle<Swapchain_t> &handle);
    OpenXrSwapchain *getSwapchain(const KDGpu::Handle<Swapchain_t> &handle) const;

    KDGpu::Handle<ActionSet_t> createActionSet(const KDGpu::Handle<Instance_t> &instanceHandle, const ActionSetOptions &options);
    void deleteActionSet(const KDGpu::Handle<ActionSet_t> &handle);
    OpenXrActionSet *getActionSet(const KDGpu::Handle<ActionSet_t> &handle) const;

    KDGpu::Handle<Action_t> createAction(const KDGpu::Handle<ActionSet_t> &actionSetHandle, const ActionOptions &options);
    void deleteAction(const KDGpu::Handle<Action_t> &handle);
    OpenXrAction *getAction(const KDGpu::Handle<Action_t> &handle) const;

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
