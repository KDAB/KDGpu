/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_export.h>
#include <KDXr/kdxr_core.h>

#include <KDGpu/handle.h>

#include <vector>

namespace KDXr {

struct ApiAction;
struct ApiActionSet;
struct ApiInstance;
struct ApiReferenceSpace;
struct ApiSession;
struct ApiSwapchain;
struct ApiSystem;

struct ActionOptions;
struct ActionSetOptions;
struct ActionSpaceOptions;
struct InstanceOptions;
struct ReferenceSpaceOptions;
struct SessionOptions;
struct SwapchainOptions;

struct Action_t;
struct ActionSet_t;
struct Instance_t;
struct ReferenceSpace_t;
struct Session_t;
struct Swapchain_t;
struct System_t;

class KDXR_EXPORT ResourceManager
{
public:
    virtual ~ResourceManager();

    virtual std::vector<ApiLayer> availableApiLayers() const;
    virtual std::vector<Extension> availableInstanceExtensions() const;

    virtual KDGpu::Handle<Instance_t> createInstance(const InstanceOptions &options) = 0;
    virtual void deleteInstance(const KDGpu::Handle<Instance_t> &handle) = 0;
    virtual ApiInstance *getInstance(const KDGpu::Handle<Instance_t> &handle) const = 0;

    virtual void removeSystem(const KDGpu::Handle<System_t> &handle) = 0;
    virtual ApiSystem *getSystem(const KDGpu::Handle<System_t> &handle) const = 0;

    virtual KDGpu::Handle<Session_t> createSession(const KDGpu::Handle<System_t> &systemHandle, const SessionOptions &options) = 0;
    virtual void deleteSession(const KDGpu::Handle<Session_t> &handle) = 0;
    virtual ApiSession *getSession(const KDGpu::Handle<Session_t> &handle) const = 0;

    virtual KDGpu::Handle<ReferenceSpace_t> createReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, const ReferenceSpaceOptions &options) = 0;
    virtual KDGpu::Handle<ReferenceSpace_t> createReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, const ActionSpaceOptions &options) = 0;
    virtual void deleteReferenceSpace(const KDGpu::Handle<ReferenceSpace_t> &handle) = 0;
    virtual ApiReferenceSpace *getReferenceSpace(const KDGpu::Handle<ReferenceSpace_t> &handle) const = 0;

    virtual KDGpu::Handle<Swapchain_t> createSwapchain(const KDGpu::Handle<Session_t> &sessionHandle, const SwapchainOptions &options) = 0;
    virtual void deleteSwapchain(const KDGpu::Handle<Swapchain_t> &handle) = 0;
    virtual ApiSwapchain *getSwapchain(const KDGpu::Handle<Swapchain_t> &handle) const = 0;

    virtual KDGpu::Handle<ActionSet_t> createActionSet(const KDGpu::Handle<Instance_t> &instanceHandle, const ActionSetOptions &options) = 0;
    virtual void deleteActionSet(const KDGpu::Handle<ActionSet_t> &handle) = 0;
    virtual ApiActionSet *getActionSet(const KDGpu::Handle<ActionSet_t> &handle) const = 0;

    virtual KDGpu::Handle<Action_t> createAction(const KDGpu::Handle<ActionSet_t> &actionSetHandle, const ActionOptions &options) = 0;
    virtual void deleteAction(const KDGpu::Handle<Action_t> &handle) = 0;
    virtual ApiAction *getAction(const KDGpu::Handle<Action_t> &handle) const = 0;

protected:
    ResourceManager();
};

} // namespace KDXr
