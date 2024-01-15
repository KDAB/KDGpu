/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/reference_space.h>

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <span>
#include <vector>

namespace KDGpu {
struct Device_t;
struct Queue_t;
class GraphicsApi;
} // namespace KDGpu

namespace KDXr {

struct Session_t;
struct System_t;
class System;
class XrApi;

/**
    @brief Holds option fields used for Session creation
    @ingroup public
    @headerfile session.h <KDXr/session.h>
 */
struct SessionOptions {
    KDGpu::GraphicsApi *graphicsApi{ nullptr };
    KDGpu::Handle<KDGpu::Device_t> device;
    uint32_t queueIndex{ 0 };
};

class KDXR_EXPORT Session
{
public:
    Session();
    ~Session();

    Session(Session &&);
    Session &operator=(Session &&);

    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;

    Handle<Session_t> handle() const noexcept { return m_session; }
    bool isValid() const { return m_session.isValid(); }

    operator Handle<Session_t>() const noexcept { return m_session; }

    ReferenceSpace createReferenceSpace(const ReferenceSpaceOptions &options = ReferenceSpaceOptions());

    std::span<const KDGpu::Format> supportedSwapchainFormats() const;

private:
    Session(const Handle<System_t> &systemHandle, XrApi *api, const SessionOptions &options);

    XrApi *m_api{ nullptr };
    Handle<System_t> m_systemHandle;
    Handle<Session_t> m_session;

    mutable std::vector<KDGpu::Format> m_supportedSwapchainFormats;

    friend class System;
};

} // namespace KDXr
