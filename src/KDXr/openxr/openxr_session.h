/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_session.h>
#include <KDXr/kdxr_core.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

namespace KDGpu {
class GraphicsApi;
struct Device_t;
} // namespace KDGpu

namespace KDXr {

class OpenXrResourceManager;
struct System_t;

/**
 * @brief OpenXrSession
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSession : public ApiSession {
    explicit OpenXrSession(OpenXrResourceManager *_openxrResourceManager,
                           XrSession _session,
                           const Handle<System_t> _systemHandle,
                           KDGpu::GraphicsApi *_graphicsApi,
                           KDGpu::Handle<KDGpu::Device_t> _device,
                           uint32_t queueIndex) noexcept;

    void initialize(Session *_frontendSession) final;
    std::vector<KDGpu::Format> supportedSwapchainFormats() const final;
    FrameState waitForFrame() final;

    void setSessionState(SessionState state);

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSession session{ XR_NULL_HANDLE };
    Handle<System_t> systemHandle;

    // Graphics related stuff
    KDGpu::GraphicsApi *graphicsApi{ nullptr };
    KDGpu::Handle<KDGpu::Device_t> deviceHandle;
    uint32_t queueIndex{ 0 };

    Session *frontendSession{ nullptr };
};

} // namespace KDXr
