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
    Handle<System_t> system;
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

private:
    Session(System *system, XrApi *api, const SessionOptions &options);

    XrApi *m_api{ nullptr };
    System *m_system{ nullptr };
    Handle<Session_t> m_session;

    friend class System;
};

} // namespace KDXr
