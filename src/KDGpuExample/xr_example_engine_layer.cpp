/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_example_engine_layer.h"

#include <KDGpuExample/engine.h>

#include <KDGui/gui_application.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

namespace KDGpuExample {

XrExampleEngineLayer::XrExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
    m_logger = spdlog::get("engine");
    if (!m_logger)
        m_logger = spdlog::stdout_color_mt("engine");
}

XrExampleEngineLayer::~XrExampleEngineLayer()
{
}

void XrExampleEngineLayer::onAttached()
{
    // OpenXR Setup
    createXrInstance();

    // Vulkan Setup

    // Request an instance of the api with whatever layers and extensions we wish to request.
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->applicationName(),
        .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);
}

void XrExampleEngineLayer::onDetached()
{
    destroyXrInstance();
}

void XrExampleEngineLayer::update()
{
}

void XrExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
}

void XrExampleEngineLayer::createXrInstance()
{
    XrApplicationInfo xrApplicationInfo = {};
    const auto appName = KDGui::GuiApplication::instance()->applicationName();
    strncpy(xrApplicationInfo.applicationName, appName.data(), XR_MAX_APPLICATION_NAME_SIZE);
    xrApplicationInfo.applicationVersion = 1;
    strncpy(xrApplicationInfo.engineName, "KDGpuExample XR Engine", XR_MAX_ENGINE_NAME_SIZE);
    xrApplicationInfo.engineVersion = 1;
    xrApplicationInfo.apiVersion = XR_CURRENT_API_VERSION;

    std::vector<std::string> instanceExtensions = { XR_EXT_DEBUG_UTILS_EXTENSION_NAME, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME };

    // Query and check layers
    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    if (xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate ApiLayerProperties.");
        return;
    }
    apiLayerProperties.resize(apiLayerCount, { XR_TYPE_API_LAYER_PROPERTIES });
    if (xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate ApiLayerProperties.");
        return;
    }

    // Check the requested API layers against the ones enumerated from OpenXR. If found add it to the active api Layers.
    for (auto &requestedLayer : m_xrRequestedApiLayers) {
        for (auto &layerProperty : apiLayerProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestedLayer.c_str(), layerProperty.layerName) != 0) {
                continue;
            } else {
                m_xrActiveApiLayers.push_back(requestedLayer.c_str());
                break;
            }
        }
    }

    // Query and check instance extensions
    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> extensionProperties;
    if (xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate InstanceExtensionProperties.");
        return;
    }
    extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
    if (xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate InstanceExtensionProperties.");
        return;
    }

    for (auto &requestedInstanceExtension : m_xrRequestedInstanceExtensions) {
        bool found = false;
        for (auto &extensionProperty : extensionProperties) {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0) {
                continue;
            } else {
                m_xrActiveInstanceExtensions.push_back(requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found) {
            SPDLOG_LOGGER_WARN(m_logger, "Failed to find requested instance extension: {}", requestedInstanceExtension);
        }
    }

    // Create the XR Instance
    XrInstanceCreateInfo instanceCI{ XR_TYPE_INSTANCE_CREATE_INFO };
    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = xrApplicationInfo;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_xrActiveApiLayers.size());
    instanceCI.enabledApiLayerNames = m_xrActiveApiLayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_xrActiveInstanceExtensions.size());
    instanceCI.enabledExtensionNames = m_xrActiveInstanceExtensions.data();
    if (xrCreateInstance(&instanceCI, &m_xrInstance) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create OpenXR Instance.");
        return;
    }
}

void XrExampleEngineLayer::destroyXrInstance()
{
}

void XrExampleEngineLayer::recreateSwapChain()
{
}

void XrExampleEngineLayer::recreateDepthTexture()
{
}

void XrExampleEngineLayer::recreateSampleDependentResources()
{
}

void XrExampleEngineLayer::uploadBufferData(const BufferUploadOptions &options)
{
}

void XrExampleEngineLayer::uploadTextureData(const TextureUploadOptions &options)
{
}

void XrExampleEngineLayer::releaseStagingBuffers()
{
}

} // namespace KDGpuExample
