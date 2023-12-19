/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_resource_manager.h"

#include <KDXr/config.h>
#include <KDXr/instance.h>

#include <cassert>
#include <stdexcept>

namespace {

bool findExtension(const std::vector<KDXr::Extension> &extensions, const std::string_view &name)
{
    const auto it = std::find_if(begin(extensions), end(extensions), [name](const KDXr::Extension &ext) { return ext.name == name; });
    return it != std::end(extensions);
};

} // namespace
namespace KDXr {

OpenXrResourceManager::OpenXrResourceManager()
{
}

OpenXrResourceManager::~OpenXrResourceManager()
{
}

Handle<Instance_t> OpenXrResourceManager::createInstance(const InstanceOptions &options)
{
    // TODO: Implement me!
    return {};
}

void OpenXrResourceManager::deleteInstance(const Handle<Instance_t> &handle)
{
    OpenXrInstance *instance = m_instances.get(handle);

    // Only destroy instances that we have allocated
    if (instance->isOwned) {

        // TODO: Destroy debug logger if we have one

        xrDestroyInstance(instance->instance);
    }

    m_instances.remove(handle);
}

OpenXrInstance *OpenXrResourceManager::getInstance(const Handle<Instance_t> &handle) const
{
    return m_instances.get(handle);
}

} // namespace KDXr
