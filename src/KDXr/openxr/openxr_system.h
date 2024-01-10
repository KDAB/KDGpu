/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_system.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <openxr/openxr.h>

namespace KDXr {

class OpenXrResourceManager;
struct Instance_t;

/**
 * @brief OpenXrSystem
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSystem : public ApiSystem {
    explicit OpenXrSystem(OpenXrResourceManager *_openxrResourceManager,
                          XrSystemId _system,
                          const Handle<Instance_t> &instanceHandle) noexcept;

    SystemProperties queryProperties() const final;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSystemId system{ XR_NULL_SYSTEM_ID };
    Handle<Instance_t> instanceHandle;
};

} // namespace KDXr
