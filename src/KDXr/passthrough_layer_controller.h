/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>

#include <KDGpu/handle.h>

#include <string>

namespace KDXr {

struct PassthroughLayer_t;
struct Session_t;
class XrApi;

/**
    @brief Holds option fields used for Passthrough Layer creation
    @ingroup public
    @headerfile passthrough_layer_controller.h <KDXr/passthrough_layer_controller.h>
 */
struct PassthroughLayerOptions {
};

class KDXR_EXPORT PassthroughLayerController
{
public:
    PassthroughLayerController();
    ~PassthroughLayerController();

    PassthroughLayerController(PassthroughLayerController &&);
    PassthroughLayerController &operator=(PassthroughLayerController &&);

    PassthroughLayerController(const PassthroughLayerController &) = delete;
    PassthroughLayerController &operator=(const PassthroughLayerController &) = delete;

    KDGpu::Handle<PassthroughLayer_t> handle() const noexcept { return m_passthroughLayer; }
    bool isValid() const { return m_passthroughLayer.isValid(); }

    operator KDGpu::Handle<PassthroughLayer_t>() const noexcept { return m_passthroughLayer; }

    void setRunning(bool running);

private:
    explicit PassthroughLayerController(const KDGpu::Handle<Session_t> &sessionHandle, XrApi *api, const PassthroughLayerOptions &options);

    XrApi *m_api{ nullptr };
    KDGpu::Handle<Session_t> m_sessionHandle;
    KDGpu::Handle<PassthroughLayer_t> m_passthroughLayer;

    friend class Session;
};

} // namespace KDXr
