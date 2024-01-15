/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_session.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrSession::OpenXrSession(OpenXrResourceManager *_openxrResourceManager,
                             XrSession _session,
                             const Handle<System_t> _systemHandle,
                             KDGpu::GraphicsApi *_graphicsApi,
                             KDGpu::Handle<KDGpu::Device_t> _device,
                             uint32_t queueIndex) noexcept
    : ApiSession()
    , openxrResourceManager(_openxrResourceManager)
    , session(_session)
    , systemHandle(_systemHandle)
    , graphicsApi(_graphicsApi)
    , deviceHandle(_device)
    , queueIndex(queueIndex)
{
}

} // namespace KDXr
