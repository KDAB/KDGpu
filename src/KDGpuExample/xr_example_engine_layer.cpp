/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "xr_example_engine_layer.h"

#include <KDGpuExample/engine.h>

#include <KDGpu/texture_options.h>

#include <KDGui/gui_application.h>

#include <assert.h>

namespace {

static XRAPI_ATTR XrBool32 XRAPI_CALL debugCallback(
        XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        XrDebugUtilsMessageTypeFlagsEXT messageType,
        const XrDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
{
    switch (messageSeverity) {
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        SPDLOG_LOGGER_DEBUG(KDGpu::Logger::logger(), "openxr message: {}", pCallbackData->message);
        break;
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        SPDLOG_LOGGER_INFO(KDGpu::Logger::logger(), "openxr message: {}", pCallbackData->message);
        break;
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        SPDLOG_LOGGER_WARN(KDGpu::Logger::logger(), "openxr message: {}", pCallbackData->message);
        break;
    case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        SPDLOG_LOGGER_ERROR(KDGpu::Logger::logger(), "openxr message: {}", pCallbackData->message);
        break;
    default:
        SPDLOG_LOGGER_TRACE(KDGpu::Logger::logger(), "openxr message: {}", pCallbackData->message);
        break;
    }

    return XR_FALSE;
}

} // namespace

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
    createXrDebugMessenger();
    getXrInstanceProperties();
    getXrSystemId();
    getXrViewConfigurations();

    // Graphics Setup
    createGraphicsInstance();
    createGraphicsDevice();

    // OpenXR Session Setup
    createXrSession();
    createXrReferenceSpace();
    createXrSwapchains();
}

void XrExampleEngineLayer::onDetached()
{
    destroyXrSwapchains();
    destroyXrReferenceSpace();
    destroyXrSession();

    destroyGraphicsDevice();
    destroyGraphicsInstance();

    destroyXrDebugMessenger();
    destroyXrInstance();
}

void XrExampleEngineLayer::update()
{
    // Release any staging buffers we are done with
    releaseStagingBuffers();

    pollXrEvents();
}

void XrExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
}

void XrExampleEngineLayer::createGraphicsInstance()
{
    // Query the minimum and maximum supported Vulkan API versions
    XrGraphicsRequirementsVulkanKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
    if (m_xrGetVulkanGraphicsRequirementsKHR(m_xrInstance, m_systemId, &graphicsRequirements) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get Graphics Requirements for Vulkan.");
        return;
    }
    SPDLOG_LOGGER_INFO(m_logger, "Minimum Vulkan API Version: {}.{}.{}",
                       XR_VERSION_MAJOR(graphicsRequirements.minApiVersionSupported),
                       XR_VERSION_MINOR(graphicsRequirements.minApiVersionSupported),
                       XR_VERSION_PATCH(graphicsRequirements.minApiVersionSupported));
    SPDLOG_LOGGER_INFO(m_logger, "Maximum Vulkan API Version: {}.{}.{}",
                       XR_VERSION_MAJOR(graphicsRequirements.maxApiVersionSupported),
                       XR_VERSION_MINOR(graphicsRequirements.maxApiVersionSupported),
                       XR_VERSION_PATCH(graphicsRequirements.maxApiVersionSupported));

    // Query the required Vulkan instance extensions
    uint32_t instanceExtensionCount = 0;
    std::vector<char> instanceExtensionProperties;
    if (m_xrGetVulkanInstanceExtensionsKHR(m_xrInstance, m_systemId, 0, &instanceExtensionCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get Vulkan Instance Extension Properties buffer size.");
        return;
    }
    instanceExtensionProperties.resize(instanceExtensionCount);
    if (m_xrGetVulkanInstanceExtensionsKHR(m_xrInstance, m_systemId, instanceExtensionCount, &instanceExtensionCount, instanceExtensionProperties.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get Vulkan Instance Extension Properties.");
        return;
    }

    // Each required instance extension is delimited by a space character.
    std::stringstream streamData(instanceExtensionProperties.data());
    std::vector<std::string> vkInstanceExtensions;
    std::string vkInstanceExtension;
    while (std::getline(streamData, vkInstanceExtension, ' ')) {
        vkInstanceExtensions.push_back(vkInstanceExtension);
        SPDLOG_LOGGER_DEBUG(m_logger, "Requesting Vulkan Instance Extension: {}", vkInstanceExtension);
    }

    // Request an instance of the api with whatever layers and extensions we wish to request.
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->applicationName(),
        .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0),
        .extensions = vkInstanceExtensions
    };
    m_instance = m_api->createInstance(instanceOptions);
}

void XrExampleEngineLayer::destroyGraphicsInstance()
{
    m_instance = {};
}

void XrExampleEngineLayer::createGraphicsDevice()
{
    // Query which physical device we should use for the given XR system
    VulkanResourceManager *vulkanResourceManager = dynamic_cast<VulkanResourceManager *>(m_api->resourceManager());
    assert(vulkanResourceManager);
    VulkanInstance *vulkanInstance = vulkanResourceManager->getInstance(m_instance);
    assert(vulkanInstance);
    VkPhysicalDevice physicalDeviceFromXR;
    if (m_xrGetVulkanGraphicsDeviceKHR(m_xrInstance, m_systemId, vulkanInstance->instance, &physicalDeviceFromXR) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get Vulkan Graphics Device from OpenXR.");
        return;
    }

    // Now look up the adapter that matches the physical device we got from OpenXR
    const auto adapters = m_instance.adapters();
    Adapter *selectedAdapter = nullptr;
    for (auto *adapter : adapters) {
        auto vulkanAdapter = vulkanResourceManager->getAdapter(adapter->handle());
        if (vulkanAdapter->physicalDevice == physicalDeviceFromXR) {
            selectedAdapter = adapter;
            break;
        }
    }
    if (!selectedAdapter) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find Adapter that matches the physical device from OpenXR.");
        return;
    }

    // Query the required Vulkan device extensions
    uint32_t deviceExtensionCount = 0;
    std::vector<char> deviceExtensionProperties;
    if (m_xrGetVulkanDeviceExtensionsKHR(m_xrInstance, m_systemId, 0, &deviceExtensionCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get Vulkan Device Extension Properties buffer size.");
        return;
    }
    deviceExtensionProperties.resize(deviceExtensionCount);
    if (m_xrGetVulkanDeviceExtensionsKHR(m_xrInstance, m_systemId, deviceExtensionCount, &deviceExtensionCount, deviceExtensionProperties.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get Vulkan Device Extension Properties.");
        return;
    }

    // Each required device extension is delimited by a space character.
    std::stringstream streamData(deviceExtensionProperties.data());
    std::vector<std::string> vkDeviceExtensions;
    std::string vkDeviceExtension;
    while (std::getline(streamData, vkDeviceExtension, ' ')) {
        vkDeviceExtensions.push_back(vkDeviceExtension);
        SPDLOG_LOGGER_DEBUG(m_logger, "Requesting Vulkan Device Extension: {}", vkDeviceExtension);
    }

    // Request a device of the api with whatever layers and extensions we wish to request.
    DeviceOptions deviceOptions = {
        .extensions = vkDeviceExtensions,
        .requestedFeatures = selectedAdapter->features()
    };
    m_device = selectedAdapter->createDevice(deviceOptions);
    m_queue = m_device.queues()[0];
}

void XrExampleEngineLayer::destroyGraphicsDevice()
{
    m_queue = {};
    m_device = {};
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

    m_xrRequestedInstanceExtensions = { XR_EXT_DEBUG_UTILS_EXTENSION_NAME, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME };
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

    // Resolve some functions we will need later
    if (xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction *)&m_xrGetVulkanGraphicsRequirementsKHR) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProcAddr for xrGetVulkanGraphicsRequirementsKHR.");
        return;
    }

    if (xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction *)&m_xrGetVulkanInstanceExtensionsKHR) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProcAddr for xrGetVulkanInstanceExtensionsKHR.");
        return;
    }

    if (xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction *)&m_xrGetVulkanDeviceExtensionsKHR) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProcAddr for xrGetVulkanDeviceExtensionsKHR.");
        return;
    }

    if (xrGetInstanceProcAddr(m_xrInstance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction *)&m_xrGetVulkanGraphicsDeviceKHR) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProcAddr for xrGetVulkanGraphicsDeviceKHR.");
        return;
    }
}

void XrExampleEngineLayer::destroyXrInstance()
{
    if (xrDestroyInstance(m_xrInstance) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to destroy OpenXR Instance.");
        return;
    }
}

void XrExampleEngineLayer::createXrDebugMessenger()
{
    bool found = false;
    for (const auto &extension : m_xrActiveInstanceExtensions) {
        if (strcmp(extension, XR_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            found = true;
            break;
        }
    }

    if (found == true) {
        XrDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{ XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        debugUtilsMessengerCreateInfo.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfo.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfo.userCallback = (PFN_xrDebugUtilsMessengerCallbackEXT)debugCallback;
        debugUtilsMessengerCreateInfo.userData = nullptr;

        // Load xrCreateDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
        PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT;
        if (xrGetInstanceProcAddr(m_xrInstance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction *)&xrCreateDebugUtilsMessengerEXT) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProcAddr.");
            return;
        }

        // Finally create and return the XrDebugUtilsMessengerEXT.
        if (xrCreateDebugUtilsMessengerEXT(m_xrInstance, &debugUtilsMessengerCreateInfo, &m_debugUtilsMessenger) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create DebugUtilsMessenger.");
            return;
        }
    }
}

void XrExampleEngineLayer::destroyXrDebugMessenger()
{
    // Load xrDestroyDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
    PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT;
    if (xrGetInstanceProcAddr(m_xrInstance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction *)&xrDestroyDebugUtilsMessengerEXT) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProcAddr.");
        return;
    }

    // Destroy the provided XrDebugUtilsMessengerEXT.
    if (xrDestroyDebugUtilsMessengerEXT(m_debugUtilsMessenger) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to destroy DebugUtilsMessenger.");
    }
}

void XrExampleEngineLayer::getXrInstanceProperties()
{
    XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
    if (xrGetInstanceProperties(m_xrInstance, &instanceProperties) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get InstanceProperties.");
        return;
    }

    SPDLOG_LOGGER_INFO(m_logger, "OpenXR Runtime: {}", instanceProperties.runtimeName);
    SPDLOG_LOGGER_INFO(m_logger, "OpenXR API Version: {}.{}.{}",
                       XR_VERSION_MAJOR(instanceProperties.runtimeVersion),
                       XR_VERSION_MINOR(instanceProperties.runtimeVersion),
                       XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void XrExampleEngineLayer::getXrSystemId()
{
    XrSystemGetInfo systemGetInfo{ XR_TYPE_SYSTEM_GET_INFO };
    systemGetInfo.formFactor = m_formFactor;
    if (const auto result = xrGetSystem(m_xrInstance, &systemGetInfo, &m_systemId) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get SystemID. Error: {}", result); // TODO: Add formatter for XrResult
        return;
    }

    // Get the System's properties for some general information about the hardware and the vendor.
    if (xrGetSystemProperties(m_xrInstance, m_systemId, &m_systemProperties) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get SystemProperties.");
        return;
    }

    SPDLOG_LOGGER_INFO(m_logger, "OpenXR System: {}", m_systemProperties.systemName);
    SPDLOG_LOGGER_INFO(m_logger, "OpenXR System Id: {}", m_systemProperties.systemId);
    SPDLOG_LOGGER_INFO(m_logger, "OpenXR Vendor Id: {}", m_systemProperties.vendorId);
    SPDLOG_LOGGER_INFO(m_logger, "Maximum swapchain dimensions: {}x{}",
                       m_systemProperties.graphicsProperties.maxSwapchainImageWidth,
                       m_systemProperties.graphicsProperties.maxSwapchainImageHeight);
    SPDLOG_LOGGER_INFO(m_logger, "Maximum layers: {}", m_systemProperties.graphicsProperties.maxLayerCount);
}

void XrExampleEngineLayer::getXrViewConfigurations()
{
    // Query the number of view configurations supported by the system
    uint32_t viewConfigurationCount = 0;
    if (xrEnumerateViewConfigurations(m_xrInstance, m_systemId, 0, &viewConfigurationCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate ViewConfigurations.");
        return;
    }

    // Query the view configurations supported by the system
    m_xrViewConfigurationTypes.resize(viewConfigurationCount);
    if (xrEnumerateViewConfigurations(m_xrInstance, m_systemId, viewConfigurationCount, &viewConfigurationCount, m_xrViewConfigurationTypes.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate ViewConfigurations.");
        return;
    }

    // Pick the first application supported View Configuration Type supported by the hardware.
    for (const XrViewConfigurationType &viewConfiguration : m_xrApplicationViewConfigurationTypes) {
        if (std::find(m_xrViewConfigurationTypes.begin(), m_xrViewConfigurationTypes.end(), viewConfiguration) != m_xrViewConfigurationTypes.end()) {
            m_xrViewConfiguration = viewConfiguration;
            break;
        }
    }
    if (m_xrViewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported ViewConfiguration. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO");
        m_xrViewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    // Query the number of environment blend modes supported by the system
    uint32_t environmentBlendModeCount = 0;
    if (xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemId, m_xrViewConfiguration, 0, &environmentBlendModeCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate EnvironmentBlendModes.");
        return;
    }

    // Query the environment blend modes supported by the system
    m_xrEnvironmentBlendModes.resize(environmentBlendModeCount);
    if (xrEnumerateEnvironmentBlendModes(m_xrInstance, m_systemId, m_xrViewConfiguration, environmentBlendModeCount, &environmentBlendModeCount, m_xrEnvironmentBlendModes.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate EnvironmentBlendModes.");
        return;
    }

    // Query the number of view configuration views in the first view configuration supported by the system
    uint32_t viewConfigurationViewCount = 0;
    if (xrEnumerateViewConfigurationViews(m_xrInstance, m_systemId, m_xrViewConfiguration, 0, &viewConfigurationViewCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate ViewConfigurationViews.");
        return;
    }

    // Query the view configuration views in the first view configuration supported by the system
    m_xrViewConfigurationViews.resize(viewConfigurationViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    if (xrEnumerateViewConfigurationViews(m_xrInstance, m_systemId, m_xrViewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, m_xrViewConfigurationViews.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate ViewConfigurationViews.");
        return;
    }
}

void XrExampleEngineLayer::createXrSession()
{
    VulkanResourceManager *vulkanResourceManager = dynamic_cast<VulkanResourceManager *>(m_api->resourceManager());
    assert(vulkanResourceManager);
    VulkanInstance *vulkanInstance = vulkanResourceManager->getInstance(m_instance);
    assert(vulkanInstance);
    VulkanAdapter *vulkanAdapter = vulkanResourceManager->getAdapter(m_device.adapter()->handle());
    assert(vulkanAdapter);
    VulkanDevice *vulkanDevice = vulkanResourceManager->getDevice(m_device);
    assert(vulkanDevice);

    XrGraphicsBindingVulkanKHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
    graphicsBinding.instance = vulkanInstance->instance;
    graphicsBinding.physicalDevice = vulkanAdapter->physicalDevice;
    graphicsBinding.device = vulkanDevice->device;
    graphicsBinding.queueFamilyIndex = m_queue.queueTypeIndex();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
    sessionCreateInfo.next = &graphicsBinding;
    sessionCreateInfo.systemId = m_systemId;

    if (xrCreateSession(m_xrInstance, &sessionCreateInfo, &m_xrSession) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create OpenXR Session.");
        return;
    }
}

void XrExampleEngineLayer::destroyXrSession()
{
    if (xrDestroySession(m_xrSession) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to destroy OpenXR Session.");
        return;
    }
}

void XrExampleEngineLayer::createXrReferenceSpace()
{
    const XrQuaternionf orientation{ 0.0f, 0.0f, 0.0f, 1.0f };
    const XrVector3f offset{ 0.0f, 0.0f, 0.0f };
    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    referenceSpaceCreateInfo.poseInReferenceSpace = { orientation, offset };
    if (xrCreateReferenceSpace(m_xrSession, &referenceSpaceCreateInfo, &m_xrReferenceSpace) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create OpenXR Reference Space.");
        return;
    }
}

void XrExampleEngineLayer::destroyXrReferenceSpace()
{
    if (xrDestroySpace(m_xrReferenceSpace) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to destroy OpenXR Reference Space.");
        return;
    }
}

void XrExampleEngineLayer::createXrSwapchains()
{
    // Query the number of swapchain formats supported by the system
    uint32_t swapchainFormatCount = 0;
    if (xrEnumerateSwapchainFormats(m_xrSession, 0, &swapchainFormatCount, nullptr) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate SwapchainFormats.");
        return;
    }

    // Query the swapchain formats supported by the system
    m_xrSwapchainFormats.resize(swapchainFormatCount);
    if (xrEnumerateSwapchainFormats(m_xrSession, swapchainFormatCount, &swapchainFormatCount, m_xrSwapchainFormats.data()) != XR_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate SwapchainFormats.");
        return;
    }

    // Note: KDGpu formats have the same value as the Vulkan formats. So if we are using the Vulkan backend we can just
    // use the KDGpu formats directly. If we are using the Metal or DX12 backend we need to convert the KDGpu formats to the
    // Metal or DX12 formats.

    // Pick the first application supported color swapchain format supported by the hardware.
    for (const auto &swapchainFormat : m_applicationColorSwapchainFormats) {
        const int64_t swapchainFormatInt = static_cast<int64_t>(swapchainFormat);
        if (std::find(m_xrSwapchainFormats.begin(), m_xrSwapchainFormats.end(), swapchainFormatInt) != m_xrSwapchainFormats.end()) {
            m_xrColorSwapchainFormat = static_cast<int64_t>(swapchainFormat);
            SPDLOG_LOGGER_INFO(m_logger, "Found a supported depth swapchain format: {}", swapchainFormatInt);
            break;
        }
    }
    if (m_xrColorSwapchainFormat == 0) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported SwapchainFormat.");
        return;
    }

    // Pick the first application supported depth swapchain format supported by the hardware.
    for (const auto &swapchainFormat : m_applicationDepthSwapchainFormats) {
        const int64_t swapchainFormatInt = static_cast<int64_t>(swapchainFormat);
        if (std::find(m_xrSwapchainFormats.begin(), m_xrSwapchainFormats.end(), swapchainFormatInt) != m_xrSwapchainFormats.end()) {
            m_xrDepthSwapchainFormat = static_cast<int64_t>(swapchainFormat);
            SPDLOG_LOGGER_INFO(m_logger, "Found a supported depth swapchain format: {}", swapchainFormatInt);
            break;
        }
    }
    if (m_xrDepthSwapchainFormat == 0) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to find a supported SwapchainFormat.");
        return;
    }

    m_colorSwapchainInfos.resize(m_xrViewConfigurationViews.size());
    m_depthSwapchainInfos.resize(m_xrViewConfigurationViews.size());

    auto vulkanApi = dynamic_cast<VulkanGraphicsApi *>(m_api.get());
    assert(vulkanApi);

    // Per view, create a color and depth swapchain, and their associated image views.
    for (size_t i = 0; i < m_xrViewConfigurationViews.size(); ++i) {
        {
            // Create a color swapchain. This will also create the underlying images which we can enumerate later.
            SwapchainInfo &colorSwapchainInfo = m_colorSwapchainInfos[i];

            XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
            swapchainCreateInfo.createFlags = 0;
            swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
            swapchainCreateInfo.format = m_xrColorSwapchainFormat;
            swapchainCreateInfo.sampleCount = m_xrViewConfigurationViews[i].recommendedSwapchainSampleCount;
            swapchainCreateInfo.width = m_xrViewConfigurationViews[i].recommendedImageRectWidth;
            swapchainCreateInfo.height = m_xrViewConfigurationViews[i].recommendedImageRectHeight;
            swapchainCreateInfo.faceCount = 1;
            swapchainCreateInfo.arraySize = 1;
            swapchainCreateInfo.mipCount = 1;
            if (xrCreateSwapchain(m_xrSession, &swapchainCreateInfo, &colorSwapchainInfo.swapchain) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create OpenXR Color Swapchain.");
                return;
            }

            // Query the number of images in the swapchain
            uint32_t swapchainImageCount = 0;
            if (xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &swapchainImageCount, nullptr) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate swapchain image count.");
                return;
            }
            SPDLOG_LOGGER_INFO(m_logger, "Color swapchain image count: {}", swapchainImageCount);

            std::vector<XrSwapchainImageVulkanKHR> swapchainImages(swapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
            if (xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, swapchainImageCount, &swapchainImageCount, reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImages.data())) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate swapchain images.");
                return;
            }

            // Convert the swapchain images to KDGpu images and create image views for them.
            colorSwapchainInfo.images.resize(swapchainImageCount);
            colorSwapchainInfo.imageViews.resize(swapchainImageCount);
            for (int j = 0; j < swapchainImageCount; ++j) {
                const TextureOptions options = {
                    .type = TextureType::TextureType2D,
                    .format = static_cast<KDGpu::Format>(m_xrColorSwapchainFormat),
                    .extent = {
                            .width = m_xrViewConfigurationViews[i].recommendedImageRectWidth,
                            .height = m_xrViewConfigurationViews[i].recommendedImageRectHeight,
                            .depth = 1 },
                    .mipLevels = 1,
                    .samples = static_cast<SampleCountFlagBits>(m_xrViewConfigurationViews[i].recommendedSwapchainSampleCount),
                    .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::ColorAttachmentBit,
                    .memoryUsage = MemoryUsage::GpuOnly
                };
                Texture texture = vulkanApi->createTextureFromExistingVkImage(m_device, options, swapchainImages[j].image);
                colorSwapchainInfo.images[j] = std::move(texture);
                colorSwapchainInfo.imageViews[j] = colorSwapchainInfo.images[j].createView();
            }
        }

        {
            // Create a depth swapchain
            SwapchainInfo &depthSwapchainInfo = m_depthSwapchainInfos[i];

            XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
            swapchainCreateInfo.createFlags = 0;
            swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            swapchainCreateInfo.format = m_xrDepthSwapchainFormat;
            swapchainCreateInfo.sampleCount = m_xrViewConfigurationViews[i].recommendedSwapchainSampleCount;
            swapchainCreateInfo.width = m_xrViewConfigurationViews[i].recommendedImageRectWidth;
            swapchainCreateInfo.height = m_xrViewConfigurationViews[i].recommendedImageRectHeight;
            swapchainCreateInfo.faceCount = 1;
            swapchainCreateInfo.arraySize = 1;
            swapchainCreateInfo.mipCount = 1;
            if (xrCreateSwapchain(m_xrSession, &swapchainCreateInfo, &depthSwapchainInfo.swapchain) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create OpenXR Depth Swapchain.");
                return;
            }

            // Query the number of images in the swapchain
            uint32_t swapchainImageCount = 0;
            if (xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &swapchainImageCount, nullptr) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate swapchain image count.");
                return;
            }
            SPDLOG_LOGGER_INFO(m_logger, "Depth swapchain image count: {}", swapchainImageCount);

            std::vector<XrSwapchainImageVulkanKHR> swapchainImages(swapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
            if (xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, swapchainImageCount, &swapchainImageCount, reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImages.data())) != XR_SUCCESS) {
                SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to enumerate swapchain images.");
                return;
            }

            // Convert the swapchain images to KDGpu images and create image views for them.
            depthSwapchainInfo.images.resize(swapchainImageCount);
            depthSwapchainInfo.imageViews.resize(swapchainImageCount);
            for (int j = 0; j < swapchainImageCount; ++j) {
                const TextureOptions options = {
                    .type = TextureType::TextureType2D,
                    .format = static_cast<KDGpu::Format>(m_xrDepthSwapchainFormat),
                    .extent = {
                            .width = m_xrViewConfigurationViews[i].recommendedImageRectWidth,
                            .height = m_xrViewConfigurationViews[i].recommendedImageRectHeight,
                            .depth = 1 },
                    .mipLevels = 1,
                    .samples = static_cast<SampleCountFlagBits>(m_xrViewConfigurationViews[i].recommendedSwapchainSampleCount),
                    .usage = TextureUsageFlagBits::SampledBit | TextureUsageFlagBits::DepthStencilAttachmentBit,
                    .memoryUsage = MemoryUsage::GpuOnly
                };
                Texture texture = vulkanApi->createTextureFromExistingVkImage(m_device, options, swapchainImages[j].image);
                depthSwapchainInfo.images[j] = std::move(texture);
                depthSwapchainInfo.imageViews[j] = depthSwapchainInfo.images[j].createView();
            }
        }
    }
}

void XrExampleEngineLayer::destroyXrSwapchains()
{
    for (auto &colorSwapchainInfo : m_colorSwapchainInfos) {
        colorSwapchainInfo.imageViews.clear();
        colorSwapchainInfo.images.clear();
        if (xrDestroySwapchain(colorSwapchainInfo.swapchain) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to destroy OpenXR Color Swapchain.");
        }
    }
    m_colorSwapchainInfos.clear();

    for (auto &depthSwapchainInfo : m_depthSwapchainInfos) {
        depthSwapchainInfo.imageViews.clear();
        depthSwapchainInfo.images.clear();
        if (xrDestroySwapchain(depthSwapchainInfo.swapchain) != XR_SUCCESS) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to destroy OpenXR Depth Swapchain.");
        }
    }
    m_depthSwapchainInfos.clear();
}

void XrExampleEngineLayer::pollXrEvents()
{
    XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
    auto XrPollEvents = [&]() -> bool {
        eventData = { XR_TYPE_EVENT_DATA_BUFFER };
        return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
    };

    while (XrPollEvents()) {
        switch (eventData.type) {
        case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
            SPDLOG_LOGGER_WARN(m_logger, "OpenXR Events Lost.");
            break;
        }

        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
            SPDLOG_LOGGER_WARN(m_logger, "OpenXR Instance Loss Pending.");
            m_xrSessionRunning = false;
            break;
        }

        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
            SPDLOG_LOGGER_INFO(m_logger, "OpenXR Interaction Profile Changed.");
            // TODO: Handle this event
            break;
        }

        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
            SPDLOG_LOGGER_INFO(m_logger, "OpenXR Reference Space Change Pending.");
            // TODO: Handle this event
            break;
        }

        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            const auto *sessionStateChanged = reinterpret_cast<const XrEventDataSessionStateChanged *>(&eventData);

            if (sessionStateChanged->session != m_xrSession) {
                SPDLOG_LOGGER_WARN(m_logger, "OpenXR Session State Changed for unknown session.");
                break;
            }

            switch (sessionStateChanged->state) {
            case XR_SESSION_STATE_READY: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Ready.");
                XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
                sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                if (xrBeginSession(m_xrSession, &sessionBeginInfo) != XR_SUCCESS) {
                    SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to begin OpenXR Session.");
                    return;
                }
                m_xrSessionRunning = true;
                break;
            }

            case XR_SESSION_STATE_SYNCHRONIZED: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Synchronized.");
                break;
            }

            case XR_SESSION_STATE_VISIBLE: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Visible.");
                break;
            }

            case XR_SESSION_STATE_FOCUSED: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Focused.");
                break;
            }

            case XR_SESSION_STATE_STOPPING: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Stopping.");
                m_xrSessionRunning = false;
                break;
            }

            case XR_SESSION_STATE_LOSS_PENDING: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Loss Pending.");
                m_xrSessionRunning = false;
                // TODO: Handle this event and exit the application or try to recreate the XrInstance and XrSession.
                break;
            }

            default: {
                SPDLOG_LOGGER_INFO(m_logger, "OpenXR Session State Changed: Unknown.");
                break;
            }
            }

            m_xrSessionState = sessionStateChanged->state;
        }
        }
    }
}

void XrExampleEngineLayer::uploadBufferData(const BufferUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadBufferData(options));
}

void XrExampleEngineLayer::uploadTextureData(const TextureUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadTextureData(options));
}

void XrExampleEngineLayer::releaseStagingBuffers()
{
    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const UploadStagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount) {
        SPDLOG_LOGGER_INFO(m_logger, "Released {} staging buffers", removedCount);
    }
}

} // namespace KDGpuExample
