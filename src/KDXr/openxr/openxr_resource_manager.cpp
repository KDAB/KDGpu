/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_resource_manager.h"

#include <KDXr/config.h>
#include <KDXr/instance.h>
#include <KDXr/utils/logging.h>

// TODO: Abstract away the choice of graphics API
#include <vulkan/vulkan.h>

#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

#include <cassert>
#include <stdexcept>

namespace {

static XRAPI_ATTR XrBool32 XRAPI_CALL debugCallback(
        XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        XrDebugUtilsMessageTypeFlagsEXT messageType,
        const XrDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
{
    switch (messageSeverity) {
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        SPDLOG_LOGGER_DEBUG(KDXr::Logger::logger(), "KDXr message: {}", pCallbackData->message);
        break;
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        SPDLOG_LOGGER_INFO(KDXr::Logger::logger(), "KDXr message: {}", pCallbackData->message);
        break;
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        SPDLOG_LOGGER_WARN(KDXr::Logger::logger(), "KDXr message: {}", pCallbackData->message);
        break;
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "KDXr message: {}", pCallbackData->message);
        break;
    default:
        SPDLOG_LOGGER_TRACE(KDXr::Logger::logger(), "KDXr message: {}", pCallbackData->message);
        break;
    }

    return XR_FALSE;
}

bool findExtension(const std::vector<KDXr::Extension> &extensions, const std::string_view &name)
{
    const auto it = std::find_if(begin(extensions), end(extensions), [name](const KDXr::Extension &ext) { return ext.name == name; });
    return it != std::end(extensions);
};

} // namespace

namespace KDXr {

OpenXrResourceManager::OpenXrResourceManager()
{
}

OpenXrResourceManager::~OpenXrResourceManager()
{
}

std::vector<ApiLayer> OpenXrResourceManager::availableApiLayers() const
{
    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    if (xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate ApiLayerProperties.");
    }
    apiLayerProperties.resize(apiLayerCount, { XR_TYPE_API_LAYER_PROPERTIES });
    if (xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate ApiLayerProperties.");
    }

    std::vector<ApiLayer> layers;
    layers.reserve(apiLayerCount);
    for (const auto &apiLayerProperty : apiLayerProperties) {
        layers.emplace_back(ApiLayer{ .name = apiLayerProperty.layerName,
                                      .description = apiLayerProperty.description,
                                      .specVersion = apiLayerProperty.specVersion,
                                      .layerVersion = apiLayerProperty.layerVersion });
    }

    return layers;
}

std::vector<Extension> OpenXrResourceManager::availableInstanceExtensions() const
{
    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> extensionProperties;
    if (xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate InstanceExtensionProperties.");
    }
    extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
    if (xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to enumerate InstanceExtensionProperties.");
    }

    std::vector<Extension> extensions;
    extensions.reserve(extensionCount);
    for (const auto &extensionProperty : extensionProperties) {
        extensions.emplace_back(Extension{ .name = extensionProperty.extensionName,
                                           .extensionVersion = extensionProperty.extensionVersion });
    }

    return extensions;
}

Handle<Instance_t> OpenXrResourceManager::createInstance(const InstanceOptions &options)
{
    XrApplicationInfo xrApplicationInfo = {};
    strncpy(xrApplicationInfo.applicationName, options.applicationName.data(), XR_MAX_APPLICATION_NAME_SIZE);
    xrApplicationInfo.applicationVersion = 1;
    strncpy(xrApplicationInfo.engineName, "KDXr Engine", XR_MAX_ENGINE_NAME_SIZE);
    xrApplicationInfo.engineVersion = 1;
    xrApplicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    // Check the requested API layers against the ones enumerated from OpenXR. If found add it to the active api Layers.
    auto apiLayers = availableApiLayers();
    std::vector<const char *> xrActiveApiLayers{}; // To pass to xrCreateInstance
    std::vector<ApiLayer> openXrActiveApiLayers; // To store in OpenXrInstance
    for (auto &requestedLayer : options.layers) {
        bool found = false;
        for (auto &layerProperty : apiLayers) {
            if (requestedLayer != layerProperty.name) {
                continue;
            } else {
                xrActiveApiLayers.push_back(requestedLayer.c_str());
                openXrActiveApiLayers.push_back(layerProperty);
                found = true;
                break;
            }
        }
        if (!found) {
            SPDLOG_LOGGER_WARN(Logger::logger(), "Failed to find requested api layer: {}", requestedLayer);
        }
    }

    // Query and check instance extensions
    auto instanceExtensions = availableInstanceExtensions();
    std::vector<const char *> xrActiveInstanceExtensions; // To pass to xrCreateInstance
    std::vector<Extension> openXrActiveExtensions; // To store in OpenXrInstance
    for (auto &requestedInstanceExtension : options.extensions) {
        bool found = false;
        for (auto &extensionProperty : instanceExtensions) {
            if (requestedInstanceExtension != extensionProperty.name) {
                continue;
            } else {
                xrActiveInstanceExtensions.push_back(requestedInstanceExtension.c_str());
                openXrActiveExtensions.push_back(extensionProperty);
                found = true;
                break;
            }
        }
        if (!found) {
            SPDLOG_LOGGER_WARN(Logger::logger(), "Failed to find requested instance extension: {}", requestedInstanceExtension);
        }
    }

    // Create the XR Instance
    XrInstanceCreateInfo instanceCI{ XR_TYPE_INSTANCE_CREATE_INFO };
    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = xrApplicationInfo;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(xrActiveApiLayers.size());
    instanceCI.enabledApiLayerNames = xrActiveApiLayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(xrActiveInstanceExtensions.size());
    instanceCI.enabledExtensionNames = xrActiveInstanceExtensions.data();

    XrInstance xrInstance{ XR_NULL_HANDLE };
    if (xrCreateInstance(&instanceCI, &xrInstance) != XR_SUCCESS) {
        throw std::runtime_error("Failed to create OpenXR Instance.");
    }

    OpenXrInstance openXrInstance(this, xrInstance, openXrActiveApiLayers, openXrActiveExtensions);

    // Create the debug logger
    bool foundDebugUtilsExtension = false;
    for (const auto &extension : xrActiveInstanceExtensions) {
        if (strcmp(extension, XR_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            foundDebugUtilsExtension = true;
            break;
        }
    }

    if (foundDebugUtilsExtension == true) {
        XrDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{ XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        debugUtilsMessengerCreateInfo.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfo.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfo.userCallback = (PFN_xrDebugUtilsMessengerCallbackEXT)debugCallback;
        debugUtilsMessengerCreateInfo.userData = nullptr;

        // Load xrCreateDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
        PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT;
        if (xrGetInstanceProcAddr(openXrInstance.instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction *)&xrCreateDebugUtilsMessengerEXT) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get InstanceProcAddr.");
        }

        // Finally create and return the XrDebugUtilsMessengerEXT.
        if (xrCreateDebugUtilsMessengerEXT != nullptr) {
            if (xrCreateDebugUtilsMessengerEXT(openXrInstance.instance, &debugUtilsMessengerCreateInfo, &openXrInstance.debugMessenger) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to create DebugUtilsMessenger.");
            }
        }
    }

    auto h = m_instances.emplace(openXrInstance);
    return h;
}

void OpenXrResourceManager::deleteInstance(const Handle<Instance_t> &handle)
{
    OpenXrInstance *openXrInstance = m_instances.get(handle);

    // Only destroy instances that we have allocated
    if (openXrInstance->isOwned) {
        // Destroy debug logger if we have one
        if (openXrInstance->debugMessenger) {
            // Load xrDestroyDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
            PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT;
            if (xrGetInstanceProcAddr(openXrInstance->instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction *)&xrDestroyDebugUtilsMessengerEXT) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get InstanceProcAddr.");
            }

            // Destroy the provided XrDebugUtilsMessengerEXT.
            if (xrDestroyDebugUtilsMessengerEXT != nullptr) {
                if (xrDestroyDebugUtilsMessengerEXT(openXrInstance->debugMessenger) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to destroy DebugUtilsMessenger.");
                }
            }
        }

        xrDestroyInstance(openXrInstance->instance);
    }

    m_instances.remove(handle);
}

OpenXrInstance *OpenXrResourceManager::getInstance(const Handle<Instance_t> &handle) const
{
    return m_instances.get(handle);
}

Handle<System_t> OpenXrResourceManager::insertSystem(const OpenXrSystem &openXrSystem)
{
    return m_systems.emplace(openXrSystem);
}

void OpenXrResourceManager::removeSystem(const Handle<System_t> &handle)
{
    m_systems.remove(handle);
}

OpenXrSystem *OpenXrResourceManager::getSystem(const Handle<System_t> &handle) const
{
    return m_systems.get(handle);
}

} // namespace KDXr
