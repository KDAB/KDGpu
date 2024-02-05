/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_action_set.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

namespace KDXr {

struct Instance_t;
class OpenXrResourceManager;

/**
 * @brief OpenXrActionSet
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrActionSet : public ApiActionSet {
    explicit OpenXrActionSet(OpenXrResourceManager *_openxrResourceManager,
                             XrActionSet _actionSet,
                             const KDGpu::Handle<Instance_t> &_instanceHandle) noexcept;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrActionSet actionSet{ XR_NULL_HANDLE };
    KDGpu::Handle<Instance_t> instanceHandle;
};

} // namespace KDXr
