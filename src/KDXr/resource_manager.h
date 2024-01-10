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

struct ApiInstance;
struct ApiSystem;

struct InstanceOptions;

struct Instance_t;
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

protected:
    ResourceManager();
};

} // namespace KDXr
