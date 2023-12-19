/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_instance.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrInstance::OpenXrInstance(OpenXrResourceManager *_openxrResourceManager, XrInstance _instance, bool _isOwned) noexcept
    : ApiInstance()
    , openxrResourceManager(_openxrResourceManager)
    , instance(_instance)
    , isOwned(_isOwned)
{
}

std::vector<Extension> OpenXrInstance::extensions() const
{
    return {};
}

} // namespace KDXr
