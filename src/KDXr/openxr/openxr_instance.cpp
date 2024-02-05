/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_instance.h"

#include <KDXr/instance.h>
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

void OpenXrInstance::initialize(Instance *_frontendInstance)
{
    frontendInstance = _frontendInstance;
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

KDGpu::Handle<System_t> OpenXrInstance::querySystem(const SystemOptions &options, const KDGpu::Handle<Instance_t> &instanceHandle)
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

ProcessEventsResult OpenXrInstance::processEvents()
{
    ProcessEventsResult result{ ProcessEventsResult::Success };
    XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
    auto XrPollEvents = [&]() -> bool {
        eventData = { XR_TYPE_EVENT_DATA_BUFFER };
        result = static_cast<ProcessEventsResult>(xrPollEvent(instance, &eventData));
        return result == ProcessEventsResult::Success;
    };

    while (XrPollEvents()) {
        switch (eventData.type) {
        case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
            SPDLOG_LOGGER_WARN(Logger::logger(), "OpenXR Events Lost.");
            break;
        }

        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
            SPDLOG_LOGGER_WARN(Logger::logger(), "OpenXR Instance Loss Pending.");
            // Find all sessions and set state to LossPending
            for (auto [xrSession, sessionHandle] : m_sessionToHandle) {
                OpenXrSession *openXrSession = openxrResourceManager->getSession(sessionHandle);
                assert(openXrSession);
                openXrSession->setSessionState(SessionState::LossPending);
            }
            frontendInstance->instanceLost.emit();
            break;
        }

        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
            SPDLOG_LOGGER_INFO(Logger::logger(), "OpenXR Interaction Profile Changed.");
            frontendInstance->interactionProfileChanged.emit();
            break;
        }

        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
            SPDLOG_LOGGER_INFO(Logger::logger(), "OpenXR Reference Space Change Pending.");
            // TODO: Handle this event
            break;
        }

        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            const auto *sessionStateChangedEvent = reinterpret_cast<const XrEventDataSessionStateChanged *>(&eventData);
            processSessionStateChangedEvent(sessionStateChangedEvent);
            break;
        }

        } // switch (eventData.type)
    } // while (XrPollEvents())

    return result;
}

void OpenXrInstance::processSessionStateChangedEvent(const XrEventDataSessionStateChanged *eventData)
{
    // Lookup the OpenXR session
    auto sessionHandle = m_sessionToHandle.find(eventData->session);
    if (sessionHandle == m_sessionToHandle.end()) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Failed to find OpenXR Session.");
        return;
    }
    OpenXrSession *openXrSession = openxrResourceManager->getSession(sessionHandle->second);
    assert(openXrSession);

    // Delegate session state changes to the OpenXR session which in turn will pass it on to the frontend session
    openXrSession->setSessionState(xrSessionStateToSessionState(eventData->state));
}

SuggestActionBindingsResult OpenXrInstance::suggestActionBindings(const SuggestActionBindingsOptions &options)
{
    std::vector<XrActionSuggestedBinding> suggestedBindings;
    suggestedBindings.reserve(options.suggestedBindings.size());
    for (const auto &suggestion : options.suggestedBindings) {
        XrPath xrPath = createXrPath(suggestion.binding);
        OpenXrAction *openXrAction = openxrResourceManager->getAction(suggestion.action);
        assert(openXrAction);
        suggestedBindings.push_back({ .action = openXrAction->action, .binding = xrPath });
    }

    XrInteractionProfileSuggestedBinding suggestedProfileBinding{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
    suggestedProfileBinding.interactionProfile = createXrPath(options.interactionProfile);
    suggestedProfileBinding.suggestedBindings = suggestedBindings.data();
    suggestedProfileBinding.countSuggestedBindings = suggestedBindings.size();

    const auto result = xrSuggestInteractionProfileBindings(instance, &suggestedProfileBinding);
    if (result != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to suggest interaction profile bindings");
    }

    return static_cast<SuggestActionBindingsResult>(result);
}

std::string OpenXrInstance::pathToString(XrPath path) const
{
    uint32_t strl;
    char text[XR_MAX_PATH_LENGTH];
    XrResult res;
    res = xrPathToString(instance, path, XR_MAX_PATH_LENGTH, &strl, text);
    std::string str;
    if (res == XR_SUCCESS) {
        str = text;
    } else {
        SPDLOG_LOGGER_CRITICAL(KDXr::Logger::logger(), "Failed to retrieve path.");
    }
    return str;
}

XrPath OpenXrInstance::createXrPath(const std::string &path) const
{
    XrPath xrPath;
    if (xrStringToPath(instance, path.c_str(), &xrPath) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(KDXr::Logger::logger(), "Failed to create XrPath.");
    }
    return xrPath;
}

} // namespace KDXr
