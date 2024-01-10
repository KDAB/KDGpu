/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_system.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrSystem::OpenXrSystem(OpenXrResourceManager *_openxrResourceManager,
                           XrSystemId _system,
                           const Handle<Instance_t> &instanceHandle) noexcept
    : ApiSystem()
    , openxrResourceManager(_openxrResourceManager)
    , system(_system)
    , instanceHandle(instanceHandle)
{
}

} // namespace KDXr
