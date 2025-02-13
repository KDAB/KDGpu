/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/config.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/passthrough_layer_controller.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

namespace KDXr {

struct PassthroughLayer_t;
struct Session_t;
class OpenXrResourceManager;

/**
 * @brief OpenXrPassthroughLayer
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrPassthroughLayer {
    explicit OpenXrPassthroughLayer(OpenXrResourceManager *_openxrResourceManager,
                                    XrPassthroughLayerFB _passthroughLayer,
                                    const KDGpu::Handle<Session_t> _sessionHandle,
                                    const PassthroughLayerOptions _options) noexcept;

    void setRunning(bool running);

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrPassthroughLayerFB passthroughLayer{ XR_NULL_HANDLE };
    KDGpu::Handle<Session_t> sessionHandle;
    PassthroughLayerOptions options;
};

} // namespace KDXr
