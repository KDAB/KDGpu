/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_system.h"

#include <KDXr/openxr/openxr_resource_manager.h>

#include <KDXr/utils/logging.h>

#include <KDGpu/graphics_api.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <sstream>

namespace KDXr {

OpenXrSystem::OpenXrSystem(OpenXrResourceManager *_openxrResourceManager,
                           XrSystemId _system,
                           const KDGpu::Handle<Instance_t> &instanceHandle) noexcept
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

GraphicsRequirements OpenXrSystem::queryGraphicsRequirements(KDGpu::GraphicsApi *graphicsApi) const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);

    // Determine which graphics API is in use
    if (auto vulkanGraphicsApi = dynamic_cast<KDGpu::VulkanGraphicsApi *>(graphicsApi) != nullptr) {
        // Vulkan is in use

        // Resolve needed functions if not already done so
        resolveVulkanFunctions(openxrInstance->instance);

        XrGraphicsRequirementsVulkanKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
        if (m_xrGetVulkanGraphicsRequirementsKHR(openxrInstance->instance, system, &graphicsRequirements) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get Graphics Requirements for Vulkan.");
            return {};
        }

        GraphicsRequirements requirements{
            .minApiVersionSupported = graphicsRequirements.minApiVersionSupported,
            .maxApiVersionSupported = graphicsRequirements.maxApiVersionSupported
        };
        return requirements;
    }

    SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::queryGraphicsRequirements: Unsupported graphics API.");
    return {};
}

std::vector<std::string> OpenXrSystem::requiredGraphicsInstanceExtensions(KDGpu::GraphicsApi *graphicsApi) const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);

    // Determine which graphics API is in use
    if (auto vulkanGraphicsApi = dynamic_cast<KDGpu::VulkanGraphicsApi *>(graphicsApi) != nullptr) {
        // Vulkan is in use

        // Resolve needed functions if not already done so
        resolveVulkanFunctions(openxrInstance->instance);

        // Query the required Vulkan instance extensions
        uint32_t instanceExtensionCount = 0;
        std::vector<char> instanceExtensionProperties;
        if (m_xrGetVulkanInstanceExtensionsKHR(openxrInstance->instance, system, 0, &instanceExtensionCount, nullptr) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get Vulkan Instance Extension Properties buffer size.");
            return {};
        }
        instanceExtensionProperties.resize(instanceExtensionCount);
        if (m_xrGetVulkanInstanceExtensionsKHR(openxrInstance->instance, system, instanceExtensionCount, &instanceExtensionCount, instanceExtensionProperties.data()) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get Vulkan Instance Extension Properties.");
            return {};
        }

        // Each required instance extension is delimited by a space character.
        std::stringstream streamData(instanceExtensionProperties.data());
        std::vector<std::string> vkInstanceExtensions;
        std::string vkInstanceExtension;
        while (std::getline(streamData, vkInstanceExtension, ' ')) {
            vkInstanceExtensions.push_back(vkInstanceExtension);
            SPDLOG_LOGGER_DEBUG(Logger::logger(), "Requesting Vulkan Instance Extension: {}", vkInstanceExtension);
        }

        return vkInstanceExtensions;
    }

    SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsInstanceExtensions: Unsupported graphics API.");
    return {};
}

KDGpu::Adapter *OpenXrSystem::requiredGraphicsAdapter(KDGpu::GraphicsApi *graphicsApi, const KDGpu::Instance &graphicsInstance) const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);

    // Determine which graphics API is in use
    if (auto vulkanGraphicsApi = dynamic_cast<KDGpu::VulkanGraphicsApi *>(graphicsApi) != nullptr) {
        // Vulkan is in use

        // Resolve needed functions if not already done so
        resolveVulkanFunctions(openxrInstance->instance);

        KDGpu::VulkanResourceManager *vulkanResourceManager = dynamic_cast<KDGpu::VulkanResourceManager *>(graphicsApi->resourceManager());
        assert(vulkanResourceManager);

        KDGpu::VulkanInstance *vulkanInstance = vulkanResourceManager->getInstance(graphicsInstance.handle());
        assert(vulkanInstance);

        VkPhysicalDevice physicalDeviceFromXR;
        if (m_xrGetVulkanGraphicsDeviceKHR(openxrInstance->instance, system, vulkanInstance->instance, &physicalDeviceFromXR) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get Vulkan Graphics Device from OpenXR.");
            return nullptr;
        }

        // Now look up the adapter that matches the physical device we got from OpenXR
        const auto adapters = graphicsInstance.adapters();
        KDGpu::Adapter *selectedAdapter = nullptr;
        for (auto *adapter : adapters) {
            auto vulkanAdapter = vulkanResourceManager->getAdapter(adapter->handle());
            if (vulkanAdapter->physicalDevice == physicalDeviceFromXR) {
                selectedAdapter = adapter;
                break;
            }
        }
        if (!selectedAdapter) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to find Adapter that matches the physical device from OpenXR.");
            return nullptr;
        }

        return selectedAdapter;
    }

    SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsAdapter: Unsupported graphics API.");
    return nullptr;
}

std::vector<std::string> OpenXrSystem::requiredGraphicsDeviceExtensions(KDGpu::GraphicsApi *graphicsApi) const
{
    auto openxrInstance = openxrResourceManager->getInstance(instanceHandle);

    // Determine which graphics API is in use
    if (auto vulkanGraphicsApi = dynamic_cast<KDGpu::VulkanGraphicsApi *>(graphicsApi) != nullptr) {
        // Vulkan is in use

        // Resolve needed functions if not already done so
        resolveVulkanFunctions(openxrInstance->instance);

        // Query the required Vulkan device extensions
        uint32_t deviceExtensionCount = 0;
        std::vector<char> deviceExtensionProperties;
        if (m_xrGetVulkanDeviceExtensionsKHR(openxrInstance->instance, system, 0, &deviceExtensionCount, nullptr) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get Vulkan Device Extension Properties buffer size.");
            return {};
        }
        deviceExtensionProperties.resize(deviceExtensionCount);
        if (m_xrGetVulkanDeviceExtensionsKHR(openxrInstance->instance, system, deviceExtensionCount, &deviceExtensionCount, deviceExtensionProperties.data()) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Failed to get Vulkan Device Extension Properties.");
            return {};
        }

        // Each required device extension is delimited by a space character.
        std::stringstream streamData(deviceExtensionProperties.data());
        std::vector<std::string> vkDeviceExtensions;
        std::string vkDeviceExtension;
        while (std::getline(streamData, vkDeviceExtension, ' ')) {
            vkDeviceExtensions.push_back(vkDeviceExtension);
            SPDLOG_LOGGER_DEBUG(Logger::logger(), "Requesting Vulkan Device Extension: {}", vkDeviceExtension);
        }

        return vkDeviceExtensions;
    }

    SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsDeviceExtensions: Unsupported graphics API.");
    return {};
}

void OpenXrSystem::resolveVulkanFunctions(XrInstance instance) const
{
    if (m_xrGetVulkanGraphicsRequirementsKHR && m_xrGetVulkanInstanceExtensionsKHR && m_xrGetVulkanDeviceExtensionsKHR && m_xrGetVulkanGraphicsDeviceKHR)
        return;

    if (xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction *>(&m_xrGetVulkanGraphicsRequirementsKHR)) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsInstanceExtensions: Failed to resolve xrGetVulkanGraphicsRequirementsKHR.");
        return;
    }

    if (xrGetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction *>(&m_xrGetVulkanInstanceExtensionsKHR)) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsInstanceExtensions: Failed to resolve xrGetVulkanInstanceExtensionsKHR.");
        return;
    }

    if (xrGetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction *>(&m_xrGetVulkanDeviceExtensionsKHR)) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsInstanceExtensions: Failed to resolve xrGetVulkanDeviceExtensionsKHR.");
        return;
    }

    if (xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction *>(&m_xrGetVulkanGraphicsDeviceKHR)) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "OpenXrSystem::requiredGraphicsInstanceExtensions: Failed to resolve xrGetVulkanGraphicsDeviceKHR.");
        return;
    }
}

} // namespace KDXr
