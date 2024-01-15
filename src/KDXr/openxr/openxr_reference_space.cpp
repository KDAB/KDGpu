/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_reference_space.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrReferenceSpace::OpenXrReferenceSpace(OpenXrResourceManager *_openxrResourceManager,
                                           XrSpace _referenceSpace,
                                           const Handle<Session_t> _sessionHandle,
                                           ReferenceSpaceType _type,
                                           Pose _pose) noexcept
    : ApiReferenceSpace()
    , openxrResourceManager(_openxrResourceManager)
    , referenceSpace(_referenceSpace)
    , sessionHandle(_sessionHandle)
    , type(_type)
    , pose(_pose)
{
}

} // namespace KDXr
