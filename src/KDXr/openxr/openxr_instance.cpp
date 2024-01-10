/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_instance.h"

#include <KDXr/system.h>

#include <KDXr/openxr/openxr_resource_manager.h>
#include <KDXr/openxr/openxr_enums.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

OpenXrInstance::OpenXrInstance(OpenXrResourceManager *_openxrResourceManager,
                               XrInstance _instance,
                               std::vector<ApiLayer> &_apiLayers,
                               std::vector<Extension> &_extensions,
                               bool _isOwned) noexcept
    : ApiInstance()
    , openxrResourceManager(_openxrResourceManager)
    , instance(_instance)
    , isOwned(_isOwned)
    , apiLayers(_apiLayers)
    , extensions(_extensions)
{
}

InstanceProperties OpenXrInstance::properties() const
{
    XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
    if (xrGetInstanceProperties(instance, &instanceProperties) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get InstanceProperties.");
        return {};
    }
    return { .runtimeName = instanceProperties.runtimeName,
             .runtimeVersion = instanceProperties.runtimeVersion };
}

std::vector<ApiLayer> OpenXrInstance::enabledApiLayers() const
{
    return apiLayers;
}

std::vector<Extension> OpenXrInstance::enabledExtensions() const
{
    return extensions;
}

Handle<System_t> OpenXrInstance::querySystem(const SystemOptions &options, const Handle<Instance_t> &instanceHandle)
{
    XrSystemGetInfo systemGetInfo{ XR_TYPE_SYSTEM_GET_INFO };
    systemGetInfo.formFactor = formFactorToXrFormFactor(options.formFactor);

    XrSystemId systemId{ XR_NULL_SYSTEM_ID };
    if (const auto result = xrGetSystem(instance, &systemGetInfo, &systemId) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get SystemID. Error: {}", result); // TODO: Add formatter for XrResult
        return {};
    }

    OpenXrSystem openXrSystem(openxrResourceManager, systemId, instanceHandle);
    systemHandle = openxrResourceManager->insertSystem(openXrSystem);

    return systemHandle;
}

} // namespace KDXr
