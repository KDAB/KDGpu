/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_export.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>

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

    virtual Handle<Instance_t> createInstance(const InstanceOptions &options) = 0;
    virtual void deleteInstance(const Handle<Instance_t> &handle) = 0;
    virtual ApiInstance *getInstance(const Handle<Instance_t> &handle) const = 0;

    virtual void removeSystem(const Handle<System_t> &handle) = 0;
    virtual ApiSystem *getSystem(const Handle<System_t> &handle) const = 0;

    virtual Handle<Session_t> createSession(const Handle<System_t> &systemHandle, const SessionOptions &options) = 0;
    virtual void deleteSession(const Handle<Session_t> &handle) = 0;
    virtual ApiSession *getSession(const Handle<Session_t> &handle) const = 0;

    virtual Handle<ReferenceSpace_t> createReferenceSpace(const Handle<Session_t> &sessionHandle, const ReferenceSpaceOptions &options) = 0;
    virtual void deleteReferenceSpace(const Handle<ReferenceSpace_t> &handle) = 0;
    virtual ApiReferenceSpace *getReferenceSpace(const Handle<ReferenceSpace_t> &handle) const = 0;

    virtual Handle<Swapchain_t> createSwapchain(const Handle<Session_t> &sessionHandle, const SwapchainOptions &options) = 0;
    virtual void deleteSwapchain(const Handle<Swapchain_t> &handle) = 0;
    virtual ApiSwapchain *getSwapchain(const Handle<Swapchain_t> &handle) const = 0;

    virtual Handle<ActionSet_t> createActionSet(const Handle<Instance_t> &instanceHandle, const ActionSetOptions &options) = 0;
    virtual void deleteActionSet(const Handle<ActionSet_t> &handle) = 0;
    virtual ApiActionSet *getActionSet(const Handle<ActionSet_t> &handle) const = 0;

    virtual Handle<Action_t> createAction(const Handle<ActionSet_t> &actionSetHandle, const ActionOptions &options) = 0;
    virtual void deleteAction(const Handle<Action_t> &handle) = 0;
    virtual ApiAction *getAction(const Handle<Action_t> &handle) const = 0;

protected:
    ResourceManager();
};

} // namespace KDXr
