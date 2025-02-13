/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_action_set.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrActionSet::OpenXrActionSet(OpenXrResourceManager *_openxrResourceManager,
                                 XrActionSet _actionSet,
                                 const KDGpu::Handle<Instance_t> &_instanceHandle) noexcept
    : openxrResourceManager(_openxrResourceManager)
    , actionSet(_actionSet)
    , instanceHandle(_instanceHandle)
{
}

} // namespace KDXr
