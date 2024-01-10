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

std::vector<ViewConfigurationType> OpenXrSystem::queryViewConfigurations() const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);

    std::vector<XrViewConfigurationType> xrViewConfigurationTypes;
    uint32_t viewConfigurationCount = 0;
    if (xrEnumerateViewConfigurations(openxrInstance->instance, system, 0, &viewConfigurationCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate ViewConfigurations.");
        return {};
    }

    xrViewConfigurationTypes.resize(viewConfigurationCount);
    if (xrEnumerateViewConfigurations(openxrInstance->instance, system, viewConfigurationCount, &viewConfigurationCount, xrViewConfigurationTypes.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate ViewConfigurations.");
        return {};
    }

    std::vector<ViewConfigurationType> viewConfigurationTypes;
    viewConfigurationTypes.reserve(viewConfigurationCount);
    for (const auto &viewConfigurationType : xrViewConfigurationTypes) {
        viewConfigurationTypes.push_back(static_cast<ViewConfigurationType>(viewConfigurationType));
    }

    return viewConfigurationTypes;
}

std::vector<EnvironmentBlendMode> OpenXrSystem::queryEnvironmentBlendModes(ViewConfigurationType viewConfiguration) const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    const XrViewConfigurationType xrViewConfiguration = static_cast<XrViewConfigurationType>(viewConfiguration);

    std::vector<XrEnvironmentBlendMode> xrEnvironmentBlendModes;
    uint32_t environmentBlendModeCount = 0;
    if (xrEnumerateEnvironmentBlendModes(openxrInstance->instance, system, xrViewConfiguration, 0, &environmentBlendModeCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate EnvironmentBlendModes.");
        return {};
    }

    // Query the environment blend modes supported by the system
    xrEnvironmentBlendModes.resize(environmentBlendModeCount);
    if (xrEnumerateEnvironmentBlendModes(openxrInstance->instance, system, xrViewConfiguration, environmentBlendModeCount, &environmentBlendModeCount, xrEnvironmentBlendModes.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate EnvironmentBlendModes.");
        return {};
    }

    std::vector<EnvironmentBlendMode> environmentBlendModes;
    environmentBlendModes.reserve(environmentBlendModeCount);
    for (const auto &environmentBlendMode : xrEnvironmentBlendModes) {
        environmentBlendModes.push_back(static_cast<EnvironmentBlendMode>(environmentBlendMode));
    }

    return environmentBlendModes;
}

std::vector<ViewConfigurationView> OpenXrSystem::queryViews(ViewConfigurationType viewConfiguration) const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);
    const XrViewConfigurationType xrViewConfiguration = static_cast<XrViewConfigurationType>(viewConfiguration);

    std::vector<XrViewConfigurationView> xrViewConfigurationViews;
    uint32_t viewConfigurationViewCount = 0;
    if (xrEnumerateViewConfigurationViews(openxrInstance->instance, system, xrViewConfiguration, 0, &viewConfigurationViewCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate ViewConfigurationViews.");
        return {};
    }

    // Query the view configuration views in the first view configuration supported by the system
    xrViewConfigurationViews.resize(viewConfigurationViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    if (xrEnumerateViewConfigurationViews(openxrInstance->instance, system, xrViewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, xrViewConfigurationViews.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate ViewConfigurationViews.");
        return {};
    }

    std::vector<ViewConfigurationView> viewConfigurationViews;
    viewConfigurationViews.reserve(viewConfigurationViewCount);
    for (const auto &xrViewConfigurationView : xrViewConfigurationViews) {
        viewConfigurationViews.emplace_back(ViewConfigurationView{
                .recommendedTextureWidth = xrViewConfigurationView.recommendedImageRectWidth,
                .maxTextureWidth = xrViewConfigurationView.maxImageRectWidth,
                .recommendedTextureHeight = xrViewConfigurationView.recommendedImageRectHeight,
                .maxTextureHeight = xrViewConfigurationView.maxImageRectHeight,
                .recommendedSwapchainSampleCount = xrViewConfigurationView.recommendedSwapchainSampleCount,
                .maxSwapchainSampleCount = xrViewConfigurationView.maxSwapchainSampleCount });
    }

    return viewConfigurationViews;
}

} // namespace KDXr
