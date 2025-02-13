/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_action.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrAction::OpenXrAction(OpenXrResourceManager *_openxrResourceManager,
                           XrAction _action,
                           const KDGpu::Handle<ActionSet_t> &_actionSetHandle) noexcept
    : openxrResourceManager(_openxrResourceManager)
    , actionSetHandle(_actionSetHandle)
    , action(_action)
{
}

} // namespace KDXr
