/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_reference_space.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <openxr/openxr.h>

namespace KDXr {

struct Session_t;
class OpenXrResourceManager;

/**
 * @brief OpenXrReferenceSpace
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrReferenceSpace : public ApiReferenceSpace {
    explicit OpenXrReferenceSpace(OpenXrResourceManager *_openxrResourceManager,
                                  XrSpace _referenceSpace,
                                  const Handle<Session_t> _sessionHandle,
                                  ReferenceSpaceType _type,
                                  Pose _pose) noexcept;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSpace referenceSpace{ XR_NULL_HANDLE };
    Handle<Session_t> sessionHandle;
    ReferenceSpaceType type;
    Pose pose;
};

} // namespace KDXr
