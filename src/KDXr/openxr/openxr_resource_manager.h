/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/resource_manager.h>

#include <KDXr/pool.h>

#include <KDXr/openxr/openxr_instance.h>
#include <KDXr/openxr/openxr_system.h>

#include <KDXr/kdxr_export.h>

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

    Handle<Instance_t> createInstance(const InstanceOptions &options) final;
    void deleteInstance(const Handle<Instance_t> &handle) final;
    OpenXrInstance *getInstance(const Handle<Instance_t> &handle) const final;

    Handle<System_t> insertSystem(const OpenXrSystem &openXrSystem);
    void removeSystem(const Handle<System_t> &handle) final;
    OpenXrSystem *getSystem(const Handle<System_t> &handle) const final;

private:
    Pool<OpenXrInstance, Instance_t> m_instances{ 1 };
    Pool<OpenXrSystem, System_t> m_systems{ 1 };
};

} // namespace KDXr
