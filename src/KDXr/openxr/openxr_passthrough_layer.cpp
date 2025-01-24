/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_passthrough_layer.h"

#include <KDXr/openxr/openxr_enums.h>
#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrPassthroughLayer::OpenXrPassthroughLayer(OpenXrResourceManager *_openxrResourceManager,
                                               XrPassthroughLayerFB _passthroughLayer,
                                               const KDGpu::Handle<Session_t> _sessionHandle,
                                               const PassthroughLayerOptions _options) noexcept
    : ApiPassthroughLayer()
    , openxrResourceManager(_openxrResourceManager)
    , passthroughLayer(_passthroughLayer)
    , sessionHandle(_sessionHandle)
    , options(_options)
{
}

void OpenXrPassthroughLayer::setRunning(bool running)
{

    auto session = openxrResourceManager->getSession(sessionHandle);
    assert(session);
    auto *openXrInstance = openxrResourceManager->getInstance(session->instanceHandle);
    assert(openXrInstance);

    if (running) {

        PFN_xrPassthroughLayerResumeFB xrPassthroughLayerResumeFB;
        if (xrGetInstanceProcAddr(openXrInstance->instance, "xrPassthroughLayerResumeFB",
                                  (PFN_xrVoidFunction *)&xrPassthroughLayerResumeFB) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get InstanceProcAddr.");
        }

        if (xrPassthroughLayerResumeFB(passthroughLayer) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to resume passthrough playback.");
        }
    } else {
        PFN_xrPassthroughLayerResumeFB xrPassthroughLayerPauseFB;
        if (xrGetInstanceProcAddr(openXrInstance->instance, "xrPassthroughLayerPauseFB",
                                  (PFN_xrVoidFunction *)&xrPassthroughLayerPauseFB) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get InstanceProcAddr.");
        }

        if (xrPassthroughLayerPauseFB(passthroughLayer) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to pause passthrough playback.");
        }
    }
}

} // namespace KDXr
