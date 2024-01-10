/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_system.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrSystem::OpenXrSystem(OpenXrResourceManager *_openxrResourceManager,
                           XrSystemId _system,
                           const Handle<Instance_t> &instanceHandle) noexcept
    : ApiSystem()
    , openxrResourceManager(_openxrResourceManager)
    , system(_system)
    , instanceHandle(instanceHandle)
{
}

SystemProperties OpenXrSystem::queryProperties() const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);

    XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES };
    if (xrGetSystemProperties(openxrInstance->instance, system, &systemProperties) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to query system properties.");
        return {};
    }

    // clang-format off
    SystemProperties properties {
        .vendorId = systemProperties.vendorId,
        .systemName = systemProperties.systemName,
        .graphicsProperties = {
            .maxSwapchainWidth = systemProperties.graphicsProperties.maxSwapchainImageWidth,
            .maxSwapchainHeight = systemProperties.graphicsProperties.maxSwapchainImageHeight,
            .maxLayerCount = systemProperties.graphicsProperties.maxLayerCount
        },
        .trackingProperties = {
            .hasOrientationTracking = static_cast<bool>(systemProperties.trackingProperties.orientationTracking),
            .hasPositionTracking = static_cast<bool>(systemProperties.trackingProperties.positionTracking)
        }
    };
    // clang-format on

    return properties;
}

} // namespace KDXr
