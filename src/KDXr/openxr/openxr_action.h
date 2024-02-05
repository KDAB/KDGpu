/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_action.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

namespace KDXr {

struct ActionSet_t;
class OpenXrResourceManager;

/**
 * @brief OpenXrAction
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrAction : public ApiAction {
    explicit OpenXrAction(OpenXrResourceManager *_openxrResourceManager,
                          XrAction _action,
                          const KDGpu::Handle<ActionSet_t> &_actionSetHandle) noexcept;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    KDGpu::Handle<ActionSet_t> actionSetHandle;
    XrAction action{ XR_NULL_HANDLE };
};

} // namespace KDXr
