/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_export.h>
#include <KDXr/handle.h>

namespace KDXr {

struct ApiInstance;

struct InstanceOptions;

struct Instance_t;

class KDXR_EXPORT ResourceManager
{
public:
    virtual ~ResourceManager();

    virtual Handle<Instance_t> createInstance(const InstanceOptions &options) = 0;
    virtual void deleteInstance(const Handle<Instance_t> &handle) = 0;
    virtual ApiInstance *getInstance(const Handle<Instance_t> &handle) const = 0;

protected:
    ResourceManager();
};

} // namespace KDXr
