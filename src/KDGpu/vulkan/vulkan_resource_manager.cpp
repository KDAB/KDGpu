/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_resource_manager.h"
#include <KDGpu/config.h>

#if defined(KDGPU_PLATFORM_WIN32)
// Avoid having to define VK_USE_PLATFORM_WIN32_KHR which would result in windows.h being included when vulkan.h is included
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <KDGpu/acceleration_structure_options.h>
#include <KDGpu/bind_group_options.h>
#include <KDGpu/bind_group_layout_options.h>
#include <KDGpu/bind_group_pool_options.h>
#include <KDGpu/buffer_options.h>
#include <KDGpu/compute_pipeline_options.h>
#include <KDGpu/graphics_pipeline_options.h>
#include <KDGpu/instance.h>
#include <KDGpu/sampler_options.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture_options.h>
#include <KDGpu/raytracing_pipeline_options.h>
#include <KDGpu/render_pass_options.h>
#include <KDGpu/vulkan/vulkan_config.h>
#include <KDGpu/vulkan/vulkan_enums.h>
#include <KDGpu/vulkan/vulkan_formatters.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>

#include <cassert>
#include <stdexcept>
#include <variant>
#include <algorithm>

namespace {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
{
    if (std::ranges::any_of(KDGpu::VulkanGraphicsApi::validationMessagesToIgnore(),
                            [pCallbackData](const std::string &error) -> bool { return pCallbackData->pMessageIdName != nullptr &&
                                                                                        error == pCallbackData->pMessageIdName; }))
        return false;

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        SPDLOG_LOGGER_DEBUG(KDGpu::Logger::logger(), "validation layer: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        SPDLOG_LOGGER_INFO(KDGpu::Logger::logger(), "validation layer: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        SPDLOG_LOGGER_WARN(KDGpu::Logger::logger(), "validation layer: {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        SPDLOG_LOGGER_ERROR(KDGpu::Logger::logger(), "validation layer: {}", pCallbackData->pMessage);
        break;
    default:
        SPDLOG_LOGGER_TRACE(KDGpu::Logger::logger(), "validation layer: {}", pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

bool hasStencilFormat(KDGpu::Format f)
{
    switch (f) {
    case KDGpu::Format::S8_UINT:
    case KDGpu::Format::D16_UNORM_S8_UINT:
    case KDGpu::Format::D24_UNORM_S8_UINT:
    case KDGpu::Format::D32_SFLOAT_S8_UINT:
        return true;
    default:
        return false;
    }
};

bool hasExtension(const std::vector<KDGpu::Extension> &extensions, const std::string_view &name)
{
    const auto it = std::find_if(extensions.begin(),
                                 extensions.end(),
                                 [name](const KDGpu::Extension &ext) { return ext.name == name; });
    return it != extensions.end();
};

bool hasExtension(const std::vector<const char *> &extensions, const char *targetExtension)
{
    const auto it = std::find_if(extensions.begin(),
                                 extensions.end(),
                                 [&](const char *ext) { return strcmp(ext, targetExtension) == 0; });
    return it != extensions.end();
}

struct SpecializationConstantData {
    uint32_t byteSize;
    std::vector<uint8_t> byteValues;
};

SpecializationConstantData getByteOffsetSizeAndRawValueForSpecializationConstant(const KDGpu::SpecializationConstant &specializationConstant)
{
    uint32_t byteSize = 0;
    std::vector<uint8_t> rawValues;

    // clang-format off
    std::visit([&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bool>) {
            byteSize = sizeof(VkBool32);
            rawValues.resize(byteSize);
            VkBool32 b(arg);
            std::memcpy(rawValues.data(), &b, byteSize);
        } else {
            byteSize = sizeof(T);
            rawValues.resize(byteSize);
            std::memcpy(rawValues.data(), &arg, byteSize);
        }
    },
    specializationConstant.value);
    // clang-format on

    return SpecializationConstantData{
        .byteSize = byteSize,
        .byteValues = std::move(rawValues),
    };
}

template<class>
inline constexpr bool always_false = false;

template<typename F>
bool testIfContainsAnyFlags(const KDGpu::Flags<F> &flags, const std::vector<F> &flagsToTest)
{
    for (const auto f : flagsToTest) {
        if (flags.testFlag(f))
            return true;
    }
    return false;
}

} // namespace
namespace KDGpu {

VulkanResourceManager::VulkanResourceManager()
{
}

VulkanResourceManager::~VulkanResourceManager()
{
}

Handle<Instance_t> VulkanResourceManager::createInstance(const InstanceOptions &options)
{
    // Populate some basic application and engine info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = options.applicationName.data();
    appInfo.applicationVersion = options.applicationVersion;
    appInfo.pEngineName = "KDGpu";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = options.apiVersion;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // On macOS we need to enable the VK_KHR_PORTABILITY_subset instance extension so that
    // the MoltenVK driver is allowed to be used even though it is technically non-conformant
    // at present. Also see vulkan_config.h. For more detail see the
    // Encountered VK_ERROR_INCOMPATIBLE_DRIVER section of
    // https://vulkan.lunarg.com/doc/sdk/1.3.216.0/mac/getting_started.html
#if defined(PLATFORM_MACOS)
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    std::vector<const char *> requestedLayers = requestedInstanceLayers;
    for (const std::string &userLayer : options.layers) {
        requestedLayers.push_back(userLayer.c_str());
    }

    std::vector<const char *> layers;

    // Query the available instance layers
    const auto availableLayers = getAvailableLayers();

    for (const char *userLayer : requestedLayers) {
        if (std::find(availableLayers.begin(), availableLayers.end(), userLayer) != availableLayers.end()) {
            layers.push_back(userLayer);
        } else {
            SPDLOG_LOGGER_WARN(Logger::logger(), "Unable to find requested layer {}", userLayer);
        }
    }

    if (!layers.empty()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        assert(requestedInstanceLayers.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledLayerNames = layers.data();
    }

    std::vector<const char *> requestedInstanceExtensions;

    // Query the available instance extensions
    const auto availableExtensions = getInstanceExtensions();

    const auto defaultRequestedExtensions = getDefaultRequestedInstanceExtensions();
    for (const char *requestedExtension : defaultRequestedExtensions) {
        if (hasExtension(availableExtensions, requestedExtension)) {
            requestedInstanceExtensions.push_back(requestedExtension);
        } else {
            SPDLOG_LOGGER_WARN(Logger::logger(), "Unable to find default requested extension {}", requestedExtension);
        }
    }

    for (const std::string &userExtension : options.extensions) {
        if (hasExtension(availableExtensions, userExtension)) {
            requestedInstanceExtensions.push_back(userExtension.c_str());
        } else {
            SPDLOG_LOGGER_WARN(Logger::logger(), "Unable to find user requested extensions {}", userExtension);
        }
    }

    if (!requestedInstanceExtensions.empty()) {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requestedInstanceExtensions.size());
        assert(requestedInstanceExtensions.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledExtensionNames = requestedInstanceExtensions.data();
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};
    const bool hasExtDebugUtilsExt = std::find_if(requestedInstanceExtensions.begin(),
                                                  requestedInstanceExtensions.end(),
                                                  [](const char *name) { return strcmp(name, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0; }) != requestedInstanceExtensions.end();
    const bool shouldRegisterDebugCallback = enableValidationLayers && hasExtDebugUtilsExt;
    if (shouldRegisterDebugCallback) {
        SPDLOG_LOGGER_INFO(Logger::logger(), "Registering Validation Debug Callback");
        // Provide the debug utils creation info to the instance creation info so it can be used during instance creation
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback = debugCallback;
        debugUtilsCreateInfo.pUserData = nullptr; // Optional

        createInfo.pNext = &debugUtilsCreateInfo;
    }

    // Try to create the instance
    VkInstance instance = VK_NULL_HANDLE;
    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error(std::string{ "Failed to create Vulkan instance: " } + getResultAsString(result));
    }

    VulkanInstance vulkanInstance(this, instance);

    // Now create the debug utils logger for ourselves (using the same callback as the instance)
    if (shouldRegisterDebugCallback) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanInstance.instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            if (func(vulkanInstance.instance, &debugUtilsCreateInfo, nullptr, &vulkanInstance.debugMessenger) != VK_SUCCESS)
                vulkanInstance.debugMessenger = nullptr;
        }
    }

    auto h = m_instances.emplace(vulkanInstance);
    return h;
}

Handle<Instance_t> VulkanResourceManager::createInstanceFromExistingVkInstance(VkInstance vkInstance)
{
    VulkanInstance vulkanInstance(this, vkInstance, false);
    auto h = m_instances.emplace(vulkanInstance);
    return h;
}

void VulkanResourceManager::deleteInstance(const Handle<Instance_t> &handle)
{
    VulkanInstance *instance = m_instances.get(handle);

    // Only destroy instances that we have allocated
    if (instance->isOwned) {

        // Destroy debug logger if we have one
        if (instance->debugMessenger) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
                func(instance->instance, instance->debugMessenger, nullptr);
        }

        vkDestroyInstance(instance->instance, nullptr);
    }

    m_instances.remove(handle);
}

VulkanInstance *VulkanResourceManager::getInstance(const Handle<Instance_t> &handle) const
{
    return m_instances.get(handle);
}

std::vector<Extension> VulkanResourceManager::getInstanceExtensions() const
{
    uint32_t extensionCount{ 0 };
    if (auto result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr); result != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to enumerate instance extensions: {}", result);
        return {};
    }

    std::vector<VkExtensionProperties> vkExtensions(extensionCount);
    if (auto result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data()); result != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to query instance extensions: {}", result);
        return {};
    }

    std::vector<Extension> extensions;
    extensions.reserve(extensionCount);
    for (const auto &vkExtension : vkExtensions) {
        extensions.emplace_back(Extension{
                .name = vkExtension.extensionName,
                .version = vkExtension.specVersion });
    }

    return extensions;
}

Handle<Adapter_t> VulkanResourceManager::insertAdapter(const VulkanAdapter &physicalDevice)
{
    return m_adapters.emplace(physicalDevice);
}

void VulkanResourceManager::removeAdapter(const Handle<Adapter_t> &handle)
{
    m_adapters.remove(handle);
}

VulkanAdapter *VulkanResourceManager::getAdapter(const Handle<Adapter_t> &handle) const
{
    return m_adapters.get(handle);
}

/*
 * Create a VkDevice (logical device) from the provided adapter (physical device) and requested options.
 * If no options are specified we request a single queue from the first family (usually graphics capable).
 */
Handle<Device_t> VulkanResourceManager::createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options, std::vector<QueueRequest> &queueRequests)
{
    VulkanAdapter *vulkanAdapter = getAdapter(adapterHandle);

    // Merge requested device extensions and layers with our defaults
    const auto availableDeviceExtensions = vulkanAdapter->extensions();
    std::vector<const char *> requestedDeviceExtensions;
    auto defaultRequestedDeviceExtensions = getDefaultRequestedDeviceExtensions();

    // Add requested device extensions set by user in the options
    for (const std::string &userRequestedExtension : options.extensions)
        defaultRequestedDeviceExtensions.push_back(userRequestedExtension.c_str());

    for (const char *requestedDeviceExtension : defaultRequestedDeviceExtensions) {
        if (hasExtension(availableDeviceExtensions, requestedDeviceExtension)) {
            requestedDeviceExtensions.push_back(requestedDeviceExtension);
        } else {
            SPDLOG_LOGGER_WARN(Logger::logger(), "Unable to find default requested device extension {}", requestedDeviceExtension);
        }
    }

    // This makes it easier to chain pNext pointers in the device createInfo struct especially when we
    // have a lot of them and some of them are optional.
    VkBaseOutStructure *chainCurrent{ nullptr };
    auto addToChain = [&chainCurrent](auto *next) {
        auto n = reinterpret_cast<VkBaseOutStructure *>(next);
        chainCurrent->pNext = n;
        chainCurrent = n;
    };

    queueRequests = options.queues;
    if (queueRequests.empty()) {
        QueueRequest queueRequest = {
            .queueTypeIndex = 0,
            .count = 1,
            .priorities = { 1.0f }
        };
        queueRequests.emplace_back(queueRequest);
    }

    uint32_t queueRequestCount = queueRequests.size();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(queueRequestCount);
    for (uint32_t i = 0; i < queueRequestCount; ++i) {
        const auto &queueRequest = queueRequests[i];

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueRequest.queueTypeIndex;
        queueCreateInfo.queueCount = queueRequest.count;
        queueCreateInfo.pQueuePriorities = queueRequest.priorities.data();

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Request the physical device features requested by options
    VkPhysicalDeviceFeatures deviceFeatures = {};
    {
        deviceFeatures.robustBufferAccess = options.requestedFeatures.robustBufferAccess;
        deviceFeatures.fullDrawIndexUint32 = options.requestedFeatures.fullDrawIndexUint32;
        deviceFeatures.imageCubeArray = options.requestedFeatures.imageCubeArray;
        deviceFeatures.independentBlend = options.requestedFeatures.independentBlend;
        deviceFeatures.geometryShader = options.requestedFeatures.geometryShader;
        deviceFeatures.tessellationShader = options.requestedFeatures.tessellationShader;
        deviceFeatures.sampleRateShading = options.requestedFeatures.sampleRateShading;
        deviceFeatures.dualSrcBlend = options.requestedFeatures.dualSrcBlend;
        deviceFeatures.logicOp = options.requestedFeatures.logicOp;
        deviceFeatures.multiDrawIndirect = options.requestedFeatures.multiDrawIndirect;
        deviceFeatures.drawIndirectFirstInstance = options.requestedFeatures.drawIndirectFirstInstance;
        deviceFeatures.depthClamp = options.requestedFeatures.depthClamp;
        deviceFeatures.depthBiasClamp = options.requestedFeatures.depthBiasClamp;
        deviceFeatures.fillModeNonSolid = options.requestedFeatures.fillModeNonSolid;
        deviceFeatures.depthBounds = options.requestedFeatures.depthBounds;
        deviceFeatures.wideLines = options.requestedFeatures.wideLines;
        deviceFeatures.largePoints = options.requestedFeatures.largePoints;
        deviceFeatures.alphaToOne = options.requestedFeatures.alphaToOne;
        deviceFeatures.multiViewport = options.requestedFeatures.multiViewport;
        deviceFeatures.samplerAnisotropy = options.requestedFeatures.samplerAnisotropy;
        deviceFeatures.textureCompressionETC2 = options.requestedFeatures.textureCompressionETC2;
        deviceFeatures.textureCompressionASTC_LDR = options.requestedFeatures.textureCompressionASTC_LDR;
        deviceFeatures.textureCompressionBC = options.requestedFeatures.textureCompressionBC;
        deviceFeatures.occlusionQueryPrecise = options.requestedFeatures.occlusionQueryPrecise;
        deviceFeatures.pipelineStatisticsQuery = options.requestedFeatures.pipelineStatisticsQuery;
        deviceFeatures.vertexPipelineStoresAndAtomics = options.requestedFeatures.vertexPipelineStoresAndAtomics;
        deviceFeatures.fragmentStoresAndAtomics = options.requestedFeatures.fragmentStoresAndAtomics;
        deviceFeatures.shaderTessellationAndGeometryPointSize = options.requestedFeatures.shaderTessellationAndGeometryPointSize;
        deviceFeatures.shaderImageGatherExtended = options.requestedFeatures.shaderImageGatherExtended;
        deviceFeatures.shaderStorageImageExtendedFormats = options.requestedFeatures.shaderStorageImageExtendedFormats;
        deviceFeatures.shaderStorageImageMultisample = options.requestedFeatures.shaderStorageImageMultisample;
        deviceFeatures.shaderStorageImageReadWithoutFormat = options.requestedFeatures.shaderStorageImageReadWithoutFormat;
        deviceFeatures.shaderStorageImageWriteWithoutFormat = options.requestedFeatures.shaderStorageImageWriteWithoutFormat;
        deviceFeatures.shaderUniformBufferArrayDynamicIndexing = options.requestedFeatures.shaderUniformBufferArrayDynamicIndexing;
        deviceFeatures.shaderSampledImageArrayDynamicIndexing = options.requestedFeatures.shaderSampledImageArrayDynamicIndexing;
        deviceFeatures.shaderStorageBufferArrayDynamicIndexing = options.requestedFeatures.shaderStorageBufferArrayDynamicIndexing;
        deviceFeatures.shaderStorageImageArrayDynamicIndexing = options.requestedFeatures.shaderStorageImageArrayDynamicIndexing;
        deviceFeatures.shaderClipDistance = options.requestedFeatures.shaderClipDistance;
        deviceFeatures.shaderCullDistance = options.requestedFeatures.shaderCullDistance;
        deviceFeatures.shaderFloat64 = options.requestedFeatures.shaderFloat64;
        deviceFeatures.shaderInt64 = options.requestedFeatures.shaderInt64;
        deviceFeatures.shaderInt16 = options.requestedFeatures.shaderInt16;
        deviceFeatures.shaderResourceResidency = options.requestedFeatures.shaderResourceResidency;
        deviceFeatures.shaderResourceMinLod = options.requestedFeatures.shaderResourceMinLod;
        deviceFeatures.sparseBinding = options.requestedFeatures.sparseBinding;
        deviceFeatures.sparseResidencyBuffer = options.requestedFeatures.sparseResidencyBuffer;
        deviceFeatures.sparseResidencyImage2D = options.requestedFeatures.sparseResidencyImage2D;
        deviceFeatures.sparseResidencyImage3D = options.requestedFeatures.sparseResidencyImage3D;
        deviceFeatures.sparseResidency2Samples = options.requestedFeatures.sparseResidency2Samples;
        deviceFeatures.sparseResidency4Samples = options.requestedFeatures.sparseResidency4Samples;
        deviceFeatures.sparseResidency8Samples = options.requestedFeatures.sparseResidency8Samples;
        deviceFeatures.sparseResidency16Samples = options.requestedFeatures.sparseResidency16Samples;
        deviceFeatures.sparseResidencyAliased = options.requestedFeatures.sparseResidencyAliased;
        deviceFeatures.variableMultisampleRate = options.requestedFeatures.variableMultisampleRate;
        deviceFeatures.inheritedQueries = options.requestedFeatures.inheritedQueries;
    }

    // Some newer features we have to request via VkPhysicalDeviceFeatures2
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.features = deviceFeatures;

    // Start the chain
    chainCurrent = reinterpret_cast<VkBaseOutStructure *>(&physicalDeviceFeatures2);

    // Allows to use std430 for uniform buffers which gives much nicer packing of data
    VkPhysicalDeviceUniformBufferStandardLayoutFeatures stdLayoutFeatures = {};
    stdLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
    stdLayoutFeatures.uniformBufferStandardLayout = options.requestedFeatures.uniformBufferStandardLayout;
    addToChain(&stdLayoutFeatures);

    // Enable multiview rendering if requested
    VkPhysicalDeviceMultiviewFeatures multiViewFeatures{};
    multiViewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
    multiViewFeatures.multiview = options.requestedFeatures.multiView;
    multiViewFeatures.multiviewGeometryShader = options.requestedFeatures.multiViewGeometryShader;
    multiViewFeatures.multiviewTessellationShader = options.requestedFeatures.multiViewTessellationShader;
    addToChain(&multiViewFeatures);

    // Enable Descriptor Indexing if requested
    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
    descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = options.requestedFeatures.shaderInputAttachmentArrayDynamicIndexing;
    descriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing = options.requestedFeatures.shaderUniformTexelBufferArrayDynamicIndexing;
    descriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing = options.requestedFeatures.shaderStorageTexelBufferArrayDynamicIndexing;
    descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = options.requestedFeatures.shaderUniformBufferArrayNonUniformIndexing;
    descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = options.requestedFeatures.shaderSampledImageArrayNonUniformIndexing;
    descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = options.requestedFeatures.shaderStorageBufferArrayNonUniformIndexing;
    descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = options.requestedFeatures.shaderStorageImageArrayNonUniformIndexing;
    descriptorIndexingFeatures.shaderInputAttachmentArrayNonUniformIndexing = options.requestedFeatures.shaderInputAttachmentArrayNonUniformIndexing;
    descriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing = options.requestedFeatures.shaderUniformTexelBufferArrayNonUniformIndexing;
    descriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing = options.requestedFeatures.shaderStorageTexelBufferArrayNonUniformIndexing;
    descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = options.requestedFeatures.bindGroupBindingUniformBufferUpdateAfterBind;
    descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = options.requestedFeatures.bindGroupBindingSampledImageUpdateAfterBind;
    descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = options.requestedFeatures.bindGroupBindingStorageImageUpdateAfterBind;
    descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = options.requestedFeatures.bindGroupBindingStorageBufferUpdateAfterBind;
    descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = options.requestedFeatures.bindGroupBindingUniformTexelBufferUpdateAfterBind;
    descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = options.requestedFeatures.bindGroupBindingStorageTexelBufferUpdateAfterBind;
    descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = options.requestedFeatures.bindGroupBindingUpdateUnusedWhilePending;
    descriptorIndexingFeatures.descriptorBindingPartiallyBound = options.requestedFeatures.bindGroupBindingPartiallyBound;
    descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = options.requestedFeatures.bindGroupBindingVariableDescriptorCount;
    descriptorIndexingFeatures.runtimeDescriptorArray = options.requestedFeatures.runtimeBindGroupArray;
    addToChain(&descriptorIndexingFeatures);

    // Create a Device that targets several physical devices if a group was specified.
    // We only add the device group info if we have more than one adapter.
    VkDeviceGroupDeviceCreateInfo deviceGroupInfo = {};
    deviceGroupInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO_KHR;
    deviceGroupInfo.physicalDeviceCount = 0;

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceFeature{};
    bufferDeviceFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceFeature.bufferDeviceAddress = options.requestedFeatures.bufferDeviceAddress;
    addToChain(&bufferDeviceFeature);

#if defined(VK_KHR_acceleration_structure)
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKhr{};
    if (options.requestedFeatures.accelerationStructures) {
        // Enable raytracing acceleration structure
        accelerationStructureFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelerationStructureFeaturesKhr.accelerationStructure = options.requestedFeatures.accelerationStructures;
        addToChain(&accelerationStructureFeaturesKhr);
    }
#endif

#if defined(VK_KHR_ray_tracing_pipeline)
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingFeaturesKhr{};

    // When running with RenderDoc (as of 1.39) it appears that options.requestedFeatures.rayTracingPipeline returns true
    // even though VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME is not present in the list of supported runtime extensions
    // (they've likely not handled all checks in their VulkanLoader)
    // Launching an application with RenderDoc then fails at device creation (returning unsupported features). To bypass
    // this, we add an extra check to see if the extension is in the list of runtime supported extensions.
    const bool deviceHasRuntimeRayTracingPipelineExtension = hasExtension(requestedDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    if (options.requestedFeatures.rayTracingPipeline && deviceHasRuntimeRayTracingPipelineExtension) {
        // Enable raytracing pipelines
        raytracingFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        raytracingFeaturesKhr.rayTracingPipeline = options.requestedFeatures.rayTracingPipeline;
        raytracingFeaturesKhr.rayTracingPipelineShaderGroupHandleCaptureReplay = options.requestedFeatures.rayTracingPipelineShaderGroupHandleCaptureReplay;
        raytracingFeaturesKhr.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = options.requestedFeatures.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
        raytracingFeaturesKhr.rayTracingPipelineTraceRaysIndirect = options.requestedFeatures.rayTracingPipelineTraceRaysIndirect;
        raytracingFeaturesKhr.rayTraversalPrimitiveCulling = options.requestedFeatures.rayTraversalPrimitiveCulling;
        addToChain(&raytracingFeaturesKhr);
    }
#endif

#if defined(VK_EXT_mesh_shader)
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    if (options.requestedFeatures.meshShader) {
        // Enable Mesh/Task shading
        meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
        meshShaderFeatures.taskShader = options.requestedFeatures.taskShader;
        meshShaderFeatures.meshShader = options.requestedFeatures.meshShader;
        meshShaderFeatures.multiviewMeshShader = options.requestedFeatures.multiviewMeshShader;
        // Would need to enable VkPhysicalDeviceFragmentShadingRateFeaturesKHR
        // if options.requestedFeatures.primitiveFragmentShadingRateMeshShader is enabled
        meshShaderFeatures.primitiveFragmentShadingRateMeshShader = false;
        meshShaderFeatures.meshShaderQueries = options.requestedFeatures.meshShaderQueries;
        addToChain(&meshShaderFeatures);
    }
#endif

#if defined(VK_EXT_host_image_copy)
    VkPhysicalDeviceHostImageCopyFeaturesEXT hostImageCopyFeatures{};
    if (options.requestedFeatures.hostImageCopy) {
        // Enable HostImage copy
        hostImageCopyFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT;
        hostImageCopyFeatures.hostImageCopy = options.requestedFeatures.hostImageCopy;
        addToChain(&hostImageCopyFeatures);
    }
#endif

#if defined(VK_KHR_sampler_ycbcr_conversion)
    VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR ycbcrConversionFeatures{};
    if (options.requestedFeatures.samplerYCbCrConversion) {
        // Enable yCbCr sampler conversion
        ycbcrConversionFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR;
        ycbcrConversionFeatures.samplerYcbcrConversion = options.requestedFeatures.samplerYCbCrConversion;
        addToChain(&ycbcrConversionFeatures);
    }
#endif

    std::vector<VkPhysicalDevice> devicesInGroup;
    const size_t adapterCount = options.adapterGroup.adapters.size();
    const bool useDeviceGroup = adapterCount > 1;

    if (useDeviceGroup) {
        // Fetch VkPhysicalDevice from Handle<Adapter_t>
        devicesInGroup.reserve(adapterCount);
        for (const Handle<Adapter_t> &h : options.adapterGroup.adapters) {
            VulkanAdapter *adapter = getAdapter(h);
            assert(adapter);
            devicesInGroup.emplace_back(adapter->physicalDevice);
        }

        deviceGroupInfo.physicalDeviceCount = options.adapterGroup.adapters.size();
        deviceGroupInfo.pPhysicalDevices = devicesInGroup.data();

        addToChain(&deviceGroupInfo);
    }

#if defined(VK_KHR_synchronization2)
    // Enable the VK_KHR_Synchronization2 extension features by chaining this into the createInfo chain.
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features = {};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    sync2Features.synchronization2 = vulkanAdapter->supportsSynchronization2;
    addToChain(&sync2Features);
#endif

#if defined(VK_KHR_dynamic_rendering)
    if (options.requestedFeatures.dynamicRendering) {
        // Enable dynamic rendering
        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
        dynamicRenderingFeatures.dynamicRendering = options.requestedFeatures.dynamicRendering;
        addToChain(&dynamicRenderingFeatures);
    }
#endif

#if defined(VK_KHR_dynamic_rendering_local_read)
    if (options.requestedFeatures.dynamicRenderingLocalRead) {
        VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicLocalReadFeatures{};
        dynamicLocalReadFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR;
        dynamicLocalReadFeatures.dynamicRenderingLocalRead = static_cast<bool>(options.requestedFeatures.dynamicRenderingLocalRead);
        addToChain(&dynamicLocalReadFeatures);
    }
#endif

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &physicalDeviceFeatures2;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = nullptr; // we use VkPhysicalDeviceFeatures2 set on pNext
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;

    // check for Vulkan API support, fall back to extensions if needed
    auto maxApiVersionSupportedByPhysicalDevice = vulkanAdapter->queryAdapterProperties().apiVersion;
    auto apiVersion = options.apiVersion;
    SPDLOG_LOGGER_INFO(
            Logger::logger(), "Requested Vulkan API Version {}.{}.{}",
            VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));
    SPDLOG_LOGGER_INFO(
            Logger::logger(), "Physical Device supports Vulkan API Version {}.{}.{}",
            VK_VERSION_MAJOR(maxApiVersionSupportedByPhysicalDevice),
            VK_VERSION_MINOR(maxApiVersionSupportedByPhysicalDevice),
            VK_VERSION_PATCH(maxApiVersionSupportedByPhysicalDevice));

    if (maxApiVersionSupportedByPhysicalDevice < apiVersion) {
        SPDLOG_LOGGER_WARN(Logger::logger(), "Downgrading requested Vulkan API Version {}.{}.{} because physical device only supports {}.{}.{}",
                           VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
                           VK_VERSION_MAJOR(maxApiVersionSupportedByPhysicalDevice),
                           VK_VERSION_MINOR(maxApiVersionSupportedByPhysicalDevice),
                           VK_VERSION_PATCH(maxApiVersionSupportedByPhysicalDevice));
        apiVersion = maxApiVersionSupportedByPhysicalDevice;
    }

#if defined(VMA_VULKAN_VERSION)
    // If we are constraining Vulkan API used by the memory allocator, for compatibility,
    // we must restrict the API version here.
#if VMA_VULKAN_VERSION < 1001000
    if (apiVersion > VK_API_VERSION_1_0) {
        apiVersion = VK_API_VERSION_1_0;
        SPDLOG_LOGGER_WARN(Logger::logger(), "Downgrading requested Vulkan API Version {}.{}.{} because VMA Allocator only supports {}.{}.{}",
                           VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
                           1, 0, 0);
    }
#elif VMA_VULKAN_VERSION < 1002000
    if (apiVersion > VK_API_VERSION_1_1) {
        apiVersion = VK_API_VERSION_1_1;
        SPDLOG_LOGGER_WARN(Logger::logger(), "Downgrading requested Vulkan API Version {}.{}.{} because VMA Allocator only supports {}.{}.{}",
                           VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
                           1, 1, 0);
    }
#elif VMA_VULKAN_VERSION < 1003000
    if (apiVersion > VK_API_VERSION_1_2) {
        apiVersion = VK_API_VERSION_1_2;
        SPDLOG_LOGGER_WARN(Logger::logger(), "Downgrading requested Vulkan API Version {}.{}.{} because VMA Allocator only supports {}.{}.{}",
                           VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
                           1, 2, 0);
    }
#endif
#endif

    const bool hasVulkan12 = apiVersion >= VK_API_VERSION_1_2;
    const bool hasVulkan11 = apiVersion >= VK_API_VERSION_1_1;

    if (!hasVulkan12 && hasVulkan11) {
        SPDLOG_LOGGER_INFO(Logger::logger(), "Vulkan 1.2 is unavailable, falling back to Vulkan 1.1...");
        std::vector<const char *> vulkan11Extensions = {
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
            VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
        };
        for (const char *requestedVulkan11Extension : vulkan11Extensions) {
            if (hasExtension(availableDeviceExtensions, requestedVulkan11Extension)) {
                requestedDeviceExtensions.push_back(requestedVulkan11Extension);
            } else {
                SPDLOG_LOGGER_WARN(Logger::logger(), "Unable to find default requested Vulkan 1.1 extension {}", requestedVulkan11Extension);
            }
        }

    } else if (!hasVulkan12 && !hasVulkan11) {
        throw std::runtime_error("At least Vulkan 1.1 is required!");
    }

    if (!requestedDeviceExtensions.empty()) {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requestedDeviceExtensions.size());
        assert(requestedDeviceExtensions.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data();
    }

    VkDevice vkDevice{ VK_NULL_HANDLE };
    VkResult result = vkCreateDevice(vulkanAdapter->physicalDevice, &createInfo, nullptr, &vkDevice);
    if (result != VK_SUCCESS)
        throw std::runtime_error(std::string{ "Failed to create a logical device: " } + getResultAsString(result));

    const auto deviceHandle = m_devices.emplace(vkDevice, apiVersion, this, adapterHandle, options.requestedFeatures);

    return deviceHandle;
}

Handle<Device_t> VulkanResourceManager::createDeviceFromExistingVkDevice(const Handle<Adapter_t> &adapterHandle, VkDevice vkDevice)
{
    VulkanAdapter *adapter = getAdapter(adapterHandle);
    assert(adapter != nullptr);
    const auto deviceHandle = m_devices.emplace(vkDevice, VK_API_VERSION_1_2, this, adapterHandle, adapter->queryAdapterFeatures(), false);

    return deviceHandle;
}

void VulkanResourceManager::deleteDevice(const Handle<Device_t> &handle)
{
    VulkanDevice *vulkanDevice = m_devices.get(handle);

    // Destroy Render Passes
    for (const auto &[passKey, passHandle] : vulkanDevice->renderPasses) {
        VulkanRenderPass *pass = m_renderPasses.get(passHandle);
        vkDestroyRenderPass(vulkanDevice->device, pass->renderPass, nullptr);
        m_renderPasses.remove(passHandle);
    }

    // Framebuffers with TextureViews attachments ought to have all been destroyed
    // since all TextureViews used as attached ought to have been destroyed by now;
    // This means we should only have imageless Framebuffers left

    // Destroy imageless FrameBuffers
    for (const auto &[fbKey, fbHandle] : vulkanDevice->framebuffers) {
        assert(fbKey.attachmentsKey.handles.empty());
        deleteFramebuffer(fbHandle);
    }

    // Destroy Descriptor Pools
    for (const Handle<BindGroupPool_t> &poolHandle : vulkanDevice->descriptorSetPools) {
        deleteBindGroupPool(poolHandle);
    }
    vulkanDevice->descriptorSetPools.clear();

    // Destroy Command Pool
    for (VkCommandPool commandPool : vulkanDevice->commandPools)
        vkDestroyCommandPool(vulkanDevice->device, commandPool, nullptr);

    // Destroy Timestamp Query Pool
    if (vulkanDevice->timestampQueryPool != VK_NULL_HANDLE)
        vkDestroyQueryPool(vulkanDevice->device, vulkanDevice->timestampQueryPool, nullptr);

    // Destroy Memory Allocators
    vmaDestroyAllocator(vulkanDevice->allocator);
    for (auto [memoryHandleType, externalAllocator] : vulkanDevice->externalAllocators)
        vmaDestroyAllocator(externalAllocator);

    // At last, destroy device if we allocated it
    if (vulkanDevice->isOwned)
        vkDestroyDevice(vulkanDevice->device, nullptr);

    m_devices.remove(handle);
}

VulkanDevice *VulkanResourceManager::getDevice(const Handle<Device_t> &handle) const
{
    return m_devices.get(handle);
}

Handle<Queue_t> VulkanResourceManager::insertQueue(const VulkanQueue &vulkanQueue)
{
    return m_queues.emplace(vulkanQueue);
}

void VulkanResourceManager::removeQueue(const Handle<Queue_t> &handle)
{
    m_queues.remove(handle);
}

VulkanQueue *VulkanResourceManager::getQueue(const Handle<Queue_t> &handle) const
{
    return m_queues.get(handle);
}

Handle<Swapchain_t> VulkanResourceManager::createSwapchain(const Handle<Device_t> &deviceHandle,
                                                           const SwapchainOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);
    VulkanSurface *vulkanSurface = m_surfaces.get(options.surface);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkanSurface->surface;
    createInfo.minImageCount = options.minImageCount;
    createInfo.imageFormat = formatToVkFormat(options.format);
    createInfo.imageColorSpace = colorSpaceToVkColorSpaceKHR(options.colorSpace);
    createInfo.imageExtent = { .width = options.imageExtent.width, .height = options.imageExtent.height };
    createInfo.imageArrayLayers = options.imageLayers;
    createInfo.imageUsage = options.imageUsageFlags.toInt();
    createInfo.imageSharingMode = sharingModeToVkSharingMode(options.imageSharingMode);
    if (!options.queueTypeIndices.empty()) {
        createInfo.queueFamilyIndexCount = options.queueTypeIndices.size();
        createInfo.pQueueFamilyIndices = options.queueTypeIndices.data();
    }
    createInfo.preTransform = surfaceTransformFlagBitsToVkSurfaceTransformFlagBitsKHR(options.transform);
    createInfo.compositeAlpha = compositeAlphaFlagBitsToVkCompositeAlphaFlagBitsKHR(options.compositeAlpha);
    createInfo.presentMode = presentModeToVkPresentModeKHR(options.presentMode);
    createInfo.clipped = options.clipped;

    VulkanSwapchain *oldSwapchain = m_swapchains.get(options.oldSwapchain);
    createInfo.oldSwapchain = oldSwapchain ? oldSwapchain->swapchain : VK_NULL_HANDLE;

    VkSwapchainKHR vkSwapchain{ VK_NULL_HANDLE };
    const VkResult result = vkCreateSwapchainKHR(vulkanDevice->device, &createInfo, nullptr, &vkSwapchain);
    if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating swapchain: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_SWAPCHAIN_KHR, reinterpret_cast<uint64_t>(vkSwapchain), options.label);

    const auto swapchainHandle = m_swapchains.emplace(VulkanSwapchain{
            vkSwapchain,
            options.format,
            Extent3D{ options.imageExtent.width, options.imageExtent.height, 1 },
            options.imageLayers,
            options.imageUsageFlags,
            this,
            deviceHandle });
    return swapchainHandle;
}

void VulkanResourceManager::deleteSwapchain(const Handle<Swapchain_t> &handle)
{
    VulkanSwapchain *vulkanSwapChain = m_swapchains.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanSwapChain->deviceHandle);
    vkDestroySwapchainKHR(vulkanDevice->device, vulkanSwapChain->swapchain, nullptr);
    m_swapchains.remove(handle);
}

VulkanSwapchain *VulkanResourceManager::getSwapchain(const Handle<Swapchain_t> &handle) const
{
    return m_swapchains.get(handle);
}

Handle<Surface_t> VulkanResourceManager::insertSurface(const VulkanSurface &vulkanSurface)
{
    return m_surfaces.emplace(vulkanSurface);
}

void VulkanResourceManager::deleteSurface(const Handle<Surface_t> &handle)
{
    VulkanSurface *vulkanSurface = m_surfaces.get(handle);
    if (vulkanSurface == nullptr)
        return;
    // Only destroy vkSurfaces we have allocated ourselves
    if (vulkanSurface->isOwned)
        vkDestroySurfaceKHR(vulkanSurface->instance, vulkanSurface->surface, nullptr);
    m_surfaces.remove(handle);
}

VulkanSurface *VulkanResourceManager::getSurface(const Handle<Surface_t> &handle) const
{
    return m_surfaces.get(handle);
}

Handle<Texture_t> VulkanResourceManager::insertTexture(const VulkanTexture &vulkanTexture)
{
    return m_textures.emplace(vulkanTexture);
}

void VulkanResourceManager::removeTexture(const Handle<Texture_t> &handle)
{
    m_textures.remove(handle);
}

Handle<Texture_t> VulkanResourceManager::createTexture(const Handle<Device_t> &deviceHandle, const TextureOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = textureTypeToVkImageType(options.type);
    createInfo.format = formatToVkFormat(options.format);
    createInfo.extent = {
        .width = options.extent.width,
        .height = options.extent.height,
        .depth = options.extent.depth
    };
    createInfo.mipLevels = options.mipLevels;
    createInfo.arrayLayers = options.arrayLayers;
    createInfo.samples = sampleCountFlagBitsToVkSampleFlagBits(options.samples);
    createInfo.tiling = textureTilingToVkImageTiling(options.tiling);
    createInfo.usage = options.usage.toInt();
    createInfo.sharingMode = sharingModeToVkSharingMode(options.sharingMode);
    if (!options.queueTypeIndices.empty()) {
        createInfo.queueFamilyIndexCount = options.queueTypeIndices.size();
        createInfo.pQueueFamilyIndices = options.queueTypeIndices.data();
    }
    createInfo.initialLayout = textureLayoutToVkImageLayout(options.initialLayout);

    createInfo.flags = textureCreateFlagsToVkImageCreateFlags(options.createFlags);

    if (options.type == TextureType::TextureTypeCube)
        createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageToVmaMemoryUsage(options.memoryUsage);

    VmaAllocator allocator = vulkanDevice->allocator;
    VkExternalMemoryImageCreateInfo vkExternalMemImageCreateInfo = {};
    if (options.externalMemoryHandleType != ExternalMemoryHandleTypeFlagBits::None) {
        vkExternalMemImageCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        vkExternalMemImageCreateInfo.pNext = std::exchange(createInfo.pNext, &vkExternalMemImageCreateInfo);
        vkExternalMemImageCreateInfo.handleTypes = externalMemoryHandleTypeToVkExternalMemoryHandleType(options.externalMemoryHandleType);

        // We have to use a dedicated allocator for external handles that has been created with VkExportMemoryAllocateInfo
        allocator = vulkanDevice->getOrCreateExternalMemoryAllocator(options.externalMemoryHandleType);

        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

#if defined(VK_EXT_image_drm_format_modifier)
    VkImageDrmFormatModifierListCreateInfoEXT vkImageDrmFormatModifierListCreateInfo = {};
    if (options.tiling == TextureTiling::DrmFormatModifier) {
        vkImageDrmFormatModifierListCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT;
        vkImageDrmFormatModifierListCreateInfo.pNext = std::exchange(createInfo.pNext, &vkImageDrmFormatModifierListCreateInfo);
        vkImageDrmFormatModifierListCreateInfo.drmFormatModifierCount = options.drmFormatModifiers.size();
        vkImageDrmFormatModifierListCreateInfo.pDrmFormatModifiers = options.drmFormatModifiers.data();
    }
#endif

    VkImage vkImage;
    VmaAllocation vmaAllocation;

    if (auto result = vmaCreateImage(allocator, &createInfo, &allocInfo, &vkImage, &vmaAllocation, nullptr); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating image: {}", result);
        return {};
    }

    VulkanAdapter *adapter = getAdapter(vulkanDevice->adapterHandle);
    VulkanInstance *instance = getInstance(adapter->instanceHandle);

    VmaAllocationInfo allocationInfo;
    vmaGetAllocationInfo(allocator, vmaAllocation, &allocationInfo);

    // Retrieve Shared Memory FD/Handle
    const MemoryHandle memoryHandle = (options.externalMemoryHandleType != ExternalMemoryHandleTypeFlagBits::None)
            ? retrieveExternalMemoryHandle(instance, vulkanDevice, allocationInfo, options.externalMemoryHandleType)
            : MemoryHandle{};

    uint64_t drmFormatModifier = 0;
#if defined(VK_EXT_image_drm_format_modifier)
    if (options.tiling == TextureTiling::DrmFormatModifier) {
        VkImageDrmFormatModifierPropertiesEXT modifierProps = {};
        modifierProps.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT;
        instance->vkGetImageDrmFormatModifierPropertiesEXT(vulkanDevice->device, vkImage, &modifierProps);
        drmFormatModifier = modifierProps.drmFormatModifier;
    }
#endif

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(vkImage), options.label);

    const auto vulkanTextureHandle = m_textures.emplace(VulkanTexture(
            vkImage,
            vmaAllocation,
            allocator,
            options.format,
            options.extent,
            options.mipLevels,
            options.arrayLayers,
            options.usage,
            this,
            deviceHandle,
            memoryHandle,
            drmFormatModifier));
    return vulkanTextureHandle;
}

void VulkanResourceManager::deleteTexture(const Handle<Texture_t> &handle)
{
    VulkanTexture *vulkanTexture = m_textures.get(handle);
    if (vulkanTexture->ownedBySwapchain)
        return;

    // Only destroy images we have allocated ourselves
    if (vulkanTexture->allocator && vulkanTexture->allocation) {
        vmaDestroyImage(vulkanTexture->allocator, vulkanTexture->image, vulkanTexture->allocation);
    }

    m_textures.remove(handle);
}

VulkanTexture *VulkanResourceManager::getTexture(const Handle<Texture_t> &handle) const
{
    return m_textures.get(handle);
}

Handle<TextureView_t> VulkanResourceManager::createTextureView(const Handle<Device_t> &deviceHandle,
                                                               const Handle<Texture_t> &textureHandle,
                                                               const TextureViewOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);
    VulkanTexture *vulkanTexture = m_textures.get(textureHandle);

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = vulkanTexture->image;
    createInfo.viewType = viewTypeToVkImageViewType(options.viewType);

    // Specify the format. If none specified, default to the source texture's format
    if (options.format == Format::UNDEFINED) {
        createInfo.format = formatToVkFormat(vulkanTexture->format);
    } else {
        createInfo.format = formatToVkFormat(options.format);
    }

    // Specify which subset of the texture the view exposes
    createInfo.subresourceRange = {
        .aspectMask = options.range.aspectMask.toInt(),
        .baseMipLevel = options.range.baseMipLevel,
        .levelCount = options.range.levelCount,
        .baseArrayLayer = options.range.baseArrayLayer,
        .layerCount = options.range.layerCount
    };

    // If no aspect is set, default to Color or Depth depending upon the texture usage
    if (options.range.aspectMask == TextureAspectFlagBits::None) {
        if (vulkanTexture->usage.testFlag(TextureUsageFlagBits::DepthStencilAttachmentBit))
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        else if (vulkanTexture->usage.testFlag(TextureUsageFlagBits::ColorAttachmentBit) ||
                 vulkanTexture->usage.testFlag(TextureUsageFlagBits::SampledBit)) {
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

#if defined(VK_KHR_sampler_ycbcr_conversion)
    VkSamplerYcbcrConversionKHR yCbCrConversion{ VK_NULL_HANDLE };
    VkSamplerYcbcrConversionInfoKHR yCbCrInfo{ .sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO_KHR };

    if (options.yCbCrConversion.isValid()) {
        VulkanYCbCrConversion *vulkanConversion = m_yCbCrConversions.get(options.yCbCrConversion);
        assert(vulkanConversion);
        yCbCrConversion = vulkanConversion->yCbCrConversion;
        // Set Conversion Object on Sampler
        yCbCrInfo.conversion = yCbCrConversion;
        createInfo.pNext = &yCbCrInfo;
    }
#else
    assert(!options.yCbCrConversion.isValid());
#endif

    VkImageView imageView;
    if (auto result = vkCreateImageView(vulkanDevice->device, &createInfo, nullptr, &imageView); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating image view: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64_t>(imageView), options.label);

    const auto vulkanTextureViewHandle = m_textureViews.emplace(VulkanTextureView(imageView, textureHandle, deviceHandle));
    return vulkanTextureViewHandle;
}

void VulkanResourceManager::deleteTextureView(const Handle<TextureView_t> &handle)
{
    VulkanTextureView *vulkanTextureView = m_textureViews.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanTextureView->deviceHandle);

    // Iterate over Framebuffers to destroy Framebuffers where texture view was being used as an attachment
    auto it = vulkanDevice->framebuffers.begin();
    while (it != vulkanDevice->framebuffers.end()) {
        const VulkanFramebufferKey &fbKey = it->first;
        const std::vector<Handle<TextureView_t>> &fbAttachmentHandles = fbKey.attachmentsKey.handles;
        const bool viewReferencedAsAttachment = std::ranges::find(fbAttachmentHandles, handle) != fbAttachmentHandles.end();
        if (viewReferencedAsAttachment) {
            // Destroy Framebuffer
            deleteFramebuffer(it->second);
            // Remove Framebuffer entry from the device
            it = vulkanDevice->framebuffers.erase(it);
        } else {
            ++it;
        }
    }

    vkDestroyImageView(vulkanDevice->device, vulkanTextureView->imageView, nullptr);

    m_textureViews.remove(handle);
}

VulkanTextureView *VulkanResourceManager::getTextureView(const Handle<TextureView_t> &handle) const
{
    return m_textureViews.get(handle);
}

Handle<Buffer_t> VulkanResourceManager::createBuffer(const Handle<Device_t> &deviceHandle, const BufferOptions &options, const void *initialData)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = options.size;
    createInfo.usage = options.usage.toInt();
    createInfo.sharingMode = sharingModeToVkSharingMode(options.sharingMode);
    if (!options.queueTypeIndices.empty()) {
        createInfo.queueFamilyIndexCount = options.queueTypeIndices.size();
        createInfo.pQueueFamilyIndices = options.queueTypeIndices.data();
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageToVmaMemoryUsage(options.memoryUsage);

    VmaAllocator allocator = vulkanDevice->allocator;
    VkExternalMemoryBufferCreateInfo vkExternalMemBufferCreateInfo = {};

    if (options.externalMemoryHandleType != ExternalMemoryHandleTypeFlagBits::None) {
        vkExternalMemBufferCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
        vkExternalMemBufferCreateInfo.handleTypes = externalMemoryHandleTypeToVkExternalMemoryHandleType(options.externalMemoryHandleType);
        createInfo.pNext = &vkExternalMemBufferCreateInfo;

        // We have to use a dedicated allocator for external handles that has been created with VkExportMemoryAllocateInfo
        allocator = vulkanDevice->getOrCreateExternalMemoryAllocator(options.externalMemoryHandleType);

        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

    VkBuffer vkBuffer;
    VmaAllocation vmaAllocation;

    if (auto result = vmaCreateBuffer(allocator, &createInfo, &allocInfo, &vkBuffer, &vmaAllocation, nullptr); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating buffer: {}", result);
        return {};
    }

    VulkanAdapter *adapter = getAdapter(vulkanDevice->adapterHandle);
    VulkanInstance *instance = getInstance(adapter->instanceHandle);

    VmaAllocationInfo allocationInfo;
    vmaGetAllocationInfo(allocator, vmaAllocation, &allocationInfo);

    // Retrieve Shared Memory FD/Handle
    const MemoryHandle memoryHandle = (options.externalMemoryHandleType != ExternalMemoryHandleTypeFlagBits::None)
            ? retrieveExternalMemoryHandle(instance, vulkanDevice, allocationInfo, options.externalMemoryHandleType)
            : MemoryHandle{};

    BufferDeviceAddress bufferDeviceAddress{ 0 };
    if (options.usage.testFlag(BufferUsageFlagBits::ShaderDeviceAddressBit)) {
        const VkBufferDeviceAddressInfo addressInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = vkBuffer,
        };
        bufferDeviceAddress = vkGetBufferDeviceAddress(vulkanDevice->device, &addressInfo);
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(vkBuffer), options.label);

    const auto vulkanBufferHandle = m_buffers.emplace(VulkanBuffer(vkBuffer, vmaAllocation, allocator, this, deviceHandle, memoryHandle, bufferDeviceAddress));

    if (initialData) {
        VulkanBuffer *vulkanBuffer = m_buffers.get(vulkanBufferHandle);
        auto bufferData = vulkanBuffer->map();
        std::memcpy(bufferData, initialData, createInfo.size);
        vulkanBuffer->unmap();
    }

    return vulkanBufferHandle;
}

void VulkanResourceManager::deleteBuffer(const Handle<Buffer_t> &handle)
{
    VulkanBuffer *vulkanBuffer = m_buffers.get(handle);

    vmaDestroyBuffer(vulkanBuffer->allocator, vulkanBuffer->buffer, vulkanBuffer->allocation);

    m_buffers.remove(handle);
}

VulkanBuffer *VulkanResourceManager::getBuffer(const Handle<Buffer_t> &handle) const
{
    return m_buffers.get(handle);
}

Handle<ShaderModule_t> VulkanResourceManager::createShaderModule(const Handle<Device_t> &deviceHandle, const std::vector<uint32_t> &code)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule vkShaderModule;
    if (auto result = vkCreateShaderModule(vulkanDevice->device, &createInfo, nullptr, &vkShaderModule); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating shader module: {}", result);
        return {};
    }

    const auto vulkanShaderModuleHandle = m_shaderModules.emplace(vkShaderModule, this, deviceHandle);
    return vulkanShaderModuleHandle;
}

void VulkanResourceManager::deleteShaderModule(const Handle<ShaderModule_t> &handle)
{
    VulkanShaderModule *shaderModule = m_shaderModules.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(shaderModule->deviceHandle);

    vkDestroyShaderModule(vulkanDevice->device, shaderModule->shaderModule, nullptr);

    m_shaderModules.remove(handle);
}

VulkanShaderModule *VulkanResourceManager::getShaderModule(const Handle<ShaderModule_t> &handle) const
{
    return m_shaderModules.get(handle);
}

namespace {

VkImageLayout referenceLayoutFromInputLayout(TextureLayout inputLayout)
{
    /// ensures input attachment layout are changed to read-only, as per vulkan documentation:
    /// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescription2.html
    switch (inputLayout) {
    case TextureLayout::ColorAttachmentOptimal: // VUID-VkSubpassDescription2-attachment-06912
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case TextureLayout::DepthAttachmentOptimal: // VUID-VkSubpassDescription2-attachment-06918
        return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    case TextureLayout::StencilAttachmentOptimal: // VUID-VkSubpassDescription2-attachment-06918
        return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    case TextureLayout::DepthStencilAttachmentOptimal: // VUID-VkSubpassDescription-attachment-06912
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case TextureLayout::AttachmentOptimal: // VUID-VkSubpassDescription-attachment-06921
        return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    default:
        return textureLayoutToVkImageLayout(inputLayout);
    }
}

VkImageAspectFlags imageAspectFromInputLayout(VkImageLayout layout)
{
    // aspect inference if not provided
    switch (layout) {
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default: /* should never happen. This will fail per VUID-VkSubpassDescription2-attachment-02800 */
        return VK_IMAGE_ASPECT_NONE;
    }
};

} // namespace

Handle<RenderPass_t> VulkanResourceManager::createRenderPass(const Handle<Device_t> &deviceHandle, const RenderPassOptions &options)
{
    assert(options.subpassDescriptions.size() != 0); // VUID-VkRenderPassCreateInfo2-subpassCount-arraylength

    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkRenderPassCreateInfo2 renderPassInfo = {};
    std::vector<VkAttachmentDescription2> vkAttachmentDescriptionsArray;
    std::vector<VkSubpassDescription2> vkSubpassDescriptionsArray;
    std::vector<VkSubpassDependency2> vkSubpassDependenciesArray;

    bool isMultiviewEnabled = !options.correlatedViewMasks.empty();

    for (const auto &attachmentDescription : options.attachments) {
        VkAttachmentDescription2 vkAttachment = {};
        vkAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
        vkAttachment.pNext = nullptr;
        vkAttachment.format = formatToVkFormat(attachmentDescription.format);
        vkAttachment.samples = sampleCountFlagBitsToVkSampleFlagBits(attachmentDescription.samples);
        vkAttachment.loadOp = attachmentLoadOperationToVkAttachmentLoadOp(attachmentDescription.loadOperation);
        vkAttachment.storeOp = attachmentStoreOperationToVkAttachmentStoreOp(attachmentDescription.storeOperation);
        vkAttachment.stencilLoadOp = attachmentLoadOperationToVkAttachmentLoadOp(attachmentDescription.stencilLoadOperation);
        vkAttachment.stencilStoreOp = attachmentStoreOperationToVkAttachmentStoreOp(attachmentDescription.stencilStoreOperation);
        vkAttachment.initialLayout = textureLayoutToVkImageLayout(attachmentDescription.initialLayout);
        vkAttachment.finalLayout = textureLayoutToVkImageLayout(attachmentDescription.finalLayout);

        vkAttachmentDescriptionsArray.push_back(vkAttachment);
    }

    std::vector<std::vector<VkAttachmentReference2>> inputReferenceArray;
    std::vector<std::vector<VkAttachmentReference2>> colorReferenceArray;
    std::vector<std::vector<VkAttachmentReference2>> resolveReferenceArray;
    std::vector<VkAttachmentReference2> depthReferenceArray;
    std::vector<VkAttachmentReference2> depthResolveReferenceArray;
    std::vector<VkSubpassDescriptionDepthStencilResolve> depthResolveArray;

    inputReferenceArray.reserve(options.subpassDescriptions.size());
    colorReferenceArray.reserve(options.subpassDescriptions.size());
    resolveReferenceArray.reserve(options.subpassDescriptions.size());
    depthReferenceArray.reserve(options.subpassDescriptions.size());
    depthResolveReferenceArray.reserve(options.subpassDescriptions.size());
    depthResolveArray.reserve(options.subpassDescriptions.size());

    for (const SubpassDescription &subpassDescription : options.subpassDescriptions) {
        VkSubpassDescription2 vkSubpass = {};
        vkSubpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
        vkSubpass.pNext = nullptr;
        vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        vkSubpass.inputAttachmentCount = subpassDescription.inputAttachmentReference.size();
        vkSubpass.colorAttachmentCount = subpassDescription.colorAttachmentReference.size();
        vkSubpass.preserveAttachmentCount = subpassDescription.preserveAttachmentIndex.size();
        vkSubpass.pPreserveAttachments = subpassDescription.preserveAttachmentIndex.data();

        if (isMultiviewEnabled) {
            assert(subpassDescription.viewMask != 0); // VUID-VkRenderPassCreateInfo2-viewMask-03058

            vkSubpass.viewMask = subpassDescription.viewMask;
        }

        if (subpassDescription.depthAttachmentReference) {
            VkAttachmentReference2 depthAttachment = {};

            depthAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
            depthAttachment.attachment = subpassDescription.depthAttachmentReference->index;
            if (subpassDescription.depthAttachmentReference->layout != TextureLayout::MaxEnum)
                depthAttachment.layout = textureLayoutToVkImageLayout(subpassDescription.depthAttachmentReference->layout);
            else
                depthAttachment.layout = textureLayoutToVkImageLayout(options.attachments[depthAttachment.attachment].finalLayout);

            depthReferenceArray.push_back(depthAttachment);
            vkSubpass.pDepthStencilAttachment = &depthReferenceArray.back();

            if (subpassDescription.depthResolveAttachmentReference) {
                VkAttachmentReference2 depthResolveAttachment = {};

                depthResolveAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
                depthResolveAttachment.attachment = subpassDescription.depthResolveAttachmentReference->index;
                if (subpassDescription.depthAttachmentReference->layout != TextureLayout::MaxEnum)
                    depthResolveAttachment.layout = textureLayoutToVkImageLayout(subpassDescription.depthResolveAttachmentReference->layout);
                else
                    depthResolveAttachment.layout = textureLayoutToVkImageLayout(options.attachments[depthResolveAttachment.attachment].finalLayout);

                depthResolveReferenceArray.push_back(depthResolveAttachment);

                VkSubpassDescriptionDepthStencilResolve depthResolve;
                depthResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
                depthResolve.pNext = nullptr;

                depthResolve.depthResolveMode = resolveModeToVkResolveMode(subpassDescription.depthResolveMode);
                depthResolve.stencilResolveMode = resolveModeToVkResolveMode(subpassDescription.stencilResolveMode);
                depthResolve.pDepthStencilResolveAttachment = &depthResolveReferenceArray.back();

                depthResolveArray.push_back(depthResolve);
                vkSubpass.pNext = &depthResolveArray.back();
            }
        }

        if (!subpassDescription.inputAttachmentReference.empty()) {
            std::vector<VkAttachmentReference2> inputReference;
            inputReference.reserve(vkSubpass.inputAttachmentCount);

            for (std::size_t j = 0; j < vkSubpass.inputAttachmentCount; j++) {
                VkAttachmentReference2 reference = {};
                reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
                reference.pNext = nullptr;
                reference.attachment = subpassDescription.inputAttachmentReference[j].index;

                if (subpassDescription.inputAttachmentReference[j].layout != TextureLayout::MaxEnum)
                    reference.layout = referenceLayoutFromInputLayout(subpassDescription.inputAttachmentReference[j].layout);
                else
                    reference.layout = referenceLayoutFromInputLayout(options.attachments[reference.attachment].finalLayout);

                if (!isMultiviewEnabled) {
                    const AttachmentReference &attachmentReference = subpassDescription.inputAttachmentReference[j];
                    if (attachmentReference.aspectEnabled.toInt() != 0x0) {
                        reference.aspectMask = attachmentReference.aspectEnabled.toInt();
                    } else {
                        // aspect inference if not provided
                        reference.aspectMask = imageAspectFromInputLayout(reference.layout);
                    }
                } else {
                    assert(subpassDescription.inputAttachmentAspects.size() == subpassDescription.inputAttachmentReference.size());

                    reference.aspectMask = subpassDescription.inputAttachmentAspects[j].toInt();
                }

                inputReference.push_back(reference);
            }

            inputReferenceArray.push_back(inputReference);
            vkSubpass.pInputAttachments = inputReferenceArray.back().data();
        }

        if (!subpassDescription.colorAttachmentReference.empty()) {
            std::vector<VkAttachmentReference2> colorReference;
            colorReference.reserve(vkSubpass.colorAttachmentCount);

            for (std::size_t j = 0; j < vkSubpass.colorAttachmentCount; j++) {
                VkAttachmentReference2 reference = {};

                // "aspectMask is ignored when this structure is used to describe anything other than an input attachment reference."
                reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
                reference.pNext = nullptr;
                reference.attachment = subpassDescription.colorAttachmentReference[j].index;
                if (subpassDescription.colorAttachmentReference[j].layout != TextureLayout::MaxEnum)
                    reference.layout = textureLayoutToVkImageLayout(subpassDescription.colorAttachmentReference[j].layout);
                else
                    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                colorReference.push_back(reference);
            }

            colorReferenceArray.push_back(colorReference);
            vkSubpass.pColorAttachments = colorReferenceArray.back().data();
        }

        if (!subpassDescription.resolveAttachmentReference.empty()) {
            assert(subpassDescription.resolveAttachmentReference.size() == subpassDescription.colorAttachmentReference.size());

            std::vector<VkAttachmentReference2> resolveReferences;
            resolveReferences.reserve(vkSubpass.colorAttachmentCount);

            for (std::size_t j = 0; j < vkSubpass.colorAttachmentCount; j++) {
                VkAttachmentReference2 reference = {};

                // "aspectMask is ignored when this structure is used to describe anything other than an input attachment reference."
                reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
                reference.pNext = nullptr;
                reference.attachment = subpassDescription.resolveAttachmentReference[j].index;
                if (subpassDescription.resolveAttachmentReference[j].layout != TextureLayout::MaxEnum)
                    reference.layout = textureLayoutToVkImageLayout(subpassDescription.resolveAttachmentReference[j].layout);
                else
                    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                resolveReferences.push_back(reference);
            }

            resolveReferenceArray.push_back(resolveReferences);
            vkSubpass.pResolveAttachments = resolveReferenceArray.back().data();
        }

        vkSubpassDescriptionsArray.push_back(vkSubpass);
    }

    for (const auto &subpassDependency : options.subpassDependencies) {
        VkSubpassDependency2 vkSubpassDependency = {};
        vkSubpassDependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
        vkSubpassDependency.pNext = nullptr;
        vkSubpassDependency.srcSubpass = subpassDependency.srcSubpass;
        vkSubpassDependency.dstSubpass = subpassDependency.dstSubpass;
        vkSubpassDependency.srcStageMask = pipelineStageFlagsToVkPipelineStageFlagBits(subpassDependency.srcStageMask);
        vkSubpassDependency.dstStageMask = pipelineStageFlagsToVkPipelineStageFlagBits(subpassDependency.dstStageMask);
        vkSubpassDependency.srcAccessMask = accessFlagsToVkAccessFlagBits(subpassDependency.srcAccessMask);
        vkSubpassDependency.dstAccessMask = accessFlagsToVkAccessFlagBits(subpassDependency.dstAccessMask);
        vkSubpassDependency.dependencyFlags = dependencyFlagsToVkDependencyFlags(subpassDependency.dependencyFlags);

        if (isMultiviewEnabled)
            vkSubpassDependency.viewOffset = subpassDependency.viewOffsetDependency;

        vkSubpassDependenciesArray.push_back(vkSubpassDependency);
    }

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    renderPassInfo.attachmentCount = vkAttachmentDescriptionsArray.size();
    renderPassInfo.pAttachments = (renderPassInfo.attachmentCount != 0) ? vkAttachmentDescriptionsArray.data() : nullptr;
    renderPassInfo.subpassCount = vkSubpassDescriptionsArray.size();
    renderPassInfo.pSubpasses = vkSubpassDescriptionsArray.data();
    renderPassInfo.dependencyCount = vkSubpassDependenciesArray.size();
    renderPassInfo.pDependencies = (renderPassInfo.dependencyCount != 0) ? vkSubpassDependenciesArray.data() : nullptr;

    if (isMultiviewEnabled) {
        renderPassInfo.correlatedViewMaskCount = options.correlatedViewMasks.size();
        renderPassInfo.pCorrelatedViewMasks = options.correlatedViewMasks.data();
    }

    VkRenderPass vkRenderPass{ VK_NULL_HANDLE };
    if (auto result = vulkanDevice->vkCreateRenderPass2(vulkanDevice->device, &renderPassInfo, nullptr, &vkRenderPass); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating render pass: {}", result);
        return {};
    }

    const auto vulkanRenderPassHandle = m_renderPasses.emplace(VulkanRenderPass(vkRenderPass, this, deviceHandle, options.attachments));
    return vulkanRenderPassHandle;
}

void VulkanResourceManager::deleteRenderPass(const Handle<RenderPass_t> &handle)
{
    KDGpu::VulkanRenderPass *vulkanRenderPass = m_renderPasses.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanRenderPass->deviceHandle);

    vkDestroyRenderPass(vulkanDevice->device, vulkanRenderPass->renderPass, nullptr);

    m_renderPasses.remove(handle);
}

VulkanRenderPass *VulkanResourceManager::getRenderPass(const Handle<RenderPass_t> &handle)
{
    return m_renderPasses.get(handle);
}

Handle<PipelineLayout_t> VulkanResourceManager::createPipelineLayout(const Handle<Device_t> &deviceHandle, const PipelineLayoutOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // Retrieve VkDescriptorSetLayout from the referenced options.bindGroupLayouts array
    assert(options.bindGroupLayouts.size() <= std::numeric_limits<uint32_t>::max());
    const uint32_t bindGroupLayoutCount = static_cast<uint32_t>(options.bindGroupLayouts.size());
    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
    vkDescriptorSetLayouts.reserve(bindGroupLayoutCount);

    for (uint32_t i = 0; i < bindGroupLayoutCount; ++i) {
        VulkanBindGroupLayout *bindGroupLayout = getBindGroupLayout(options.bindGroupLayouts[i]);
        vkDescriptorSetLayouts.push_back(bindGroupLayout->descriptorSetLayout);
    }

    // Create the pipeline layout
    assert(options.pushConstantRanges.size() <= std::numeric_limits<uint32_t>::max());
    const uint32_t pushConstantRangeCount = options.pushConstantRanges.size();
    std::vector<VkPushConstantRange> vkPushConstantRanges;
    vkPushConstantRanges.reserve(pushConstantRangeCount);

    for (uint32_t i = 0; i < pushConstantRangeCount; ++i) {
        const auto &pushConstantRange = options.pushConstantRanges.at(i);

        VkPushConstantRange vkPushConstantRange = {
            .stageFlags = pushConstantRange.shaderStages.toInt(),
            .offset = pushConstantRange.offset,
            .size = pushConstantRange.size
        };

        vkPushConstantRanges.emplace_back(std::move(vkPushConstantRange));
    }

    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
    createInfo.pSetLayouts = vkDescriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size());
    createInfo.pPushConstantRanges = vkPushConstantRanges.data();

    VkPipelineLayout vkPipelineLayout{ VK_NULL_HANDLE };
    if (auto result = vkCreatePipelineLayout(vulkanDevice->device, &createInfo, nullptr, &vkPipelineLayout); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating pipeline layout: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_PIPELINE_LAYOUT, reinterpret_cast<uint64_t>(vkPipelineLayout), options.label);

    // Store the results
    const auto vulkanPipelineLayoutHandle = m_pipelineLayouts.emplace(VulkanPipelineLayout(
            vkPipelineLayout,
            std::move(vkDescriptorSetLayouts),
            this,
            deviceHandle));

    return vulkanPipelineLayoutHandle;
}

void VulkanResourceManager::deletePipelineLayout(const Handle<PipelineLayout_t> &handle)
{
    VulkanPipelineLayout *vulkanPipelineLayout = m_pipelineLayouts.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanPipelineLayout->deviceHandle);

    vkDestroyPipelineLayout(vulkanDevice->device, vulkanPipelineLayout->pipelineLayout, nullptr);

    m_pipelineLayouts.remove(handle);
}

VulkanPipelineLayout *VulkanResourceManager::getPipelineLayout(const Handle<PipelineLayout_t> &handle) const
{
    return m_pipelineLayouts.get(handle);
}

Handle<GraphicsPipeline_t> VulkanResourceManager::createGraphicsPipeline(const Handle<Device_t> &deviceHandle, const GraphicsPipelineOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    if (options.dynamicRendering.enabled) {
#if !defined(VK_KHR_dynamic_rendering)
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Dynamic Rendering not supported by this Vulkan SDK");
        return {};
#endif
    }

    // Fetch the specified pipeline layout
    VulkanPipelineLayout *vulkanPipelineLayout = getPipelineLayout(options.layout);
    if (!vulkanPipelineLayout) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid pipeline layout requested");
        return {};
    }

    // Shader stages
    ShaderStagesInfo shaderStagesInfo;
    if (!fillShaderStageInfos(options.shaderStages, shaderStagesInfo)) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Failed to build shader stages info for Pipeline");
        return {};
    }

    // Vertex input
    const uint32_t vertexBindingCount = static_cast<uint32_t>(options.vertex.buffers.size());
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    vertexBindings.reserve(vertexBindingCount);
    for (uint32_t i = 0; i < vertexBindingCount; ++i) {
        const auto &binding = options.vertex.buffers.at(i);
        VkVertexInputBindingDescription vkBinding = {};
        vkBinding.binding = binding.binding;
        vkBinding.stride = binding.stride;
        vkBinding.inputRate = vertexRateToVkVertexInputRate(binding.inputRate);
        vertexBindings.emplace_back(vkBinding);
    }

    const uint32_t attributeCount = static_cast<uint32_t>(options.vertex.attributes.size());
    std::vector<VkVertexInputAttributeDescription> attributes;
    attributes.reserve(attributeCount);
    for (uint32_t i = 0; i < attributeCount; ++i) {
        const auto &attribute = options.vertex.attributes.at(i);
        VkVertexInputAttributeDescription vkAttribute = {};
        vkAttribute.location = attribute.location;
        vkAttribute.binding = attribute.binding;
        vkAttribute.format = formatToVkFormat(attribute.format);
        vkAttribute.offset = attribute.offset;
        attributes.emplace_back(vkAttribute);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
    vertexInputState.pVertexBindingDescriptions = vertexBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInputState.pVertexAttributeDescriptions = attributes.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = primitiveTopologyToVkPrimitiveTopology(options.primitive.topology);
    inputAssembly.primitiveRestartEnable = options.primitive.primitiveRestart;

    // Tessellation
    VkPipelineTessellationStateCreateInfo tessellationStateInfo = {};
    tessellationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationStateInfo.flags = 0;
    tessellationStateInfo.patchControlPoints = options.primitive.patchControlPoints;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = options.depthStencil.depthClampEnabled;
    rasterizer.rasterizerDiscardEnable = options.primitive.rasterizerDiscardEnabled;
    rasterizer.polygonMode = polygonModeToVkPolygonMode(options.primitive.polygonMode);
    rasterizer.lineWidth = options.primitive.lineWidth;
    rasterizer.cullMode = options.primitive.cullMode.toInt();
    rasterizer.frontFace = frontFaceToVkFrontFace(options.primitive.frontFace);
    rasterizer.depthBiasEnable = options.primitive.depthBias.enabled;
    rasterizer.depthBiasConstantFactor = options.primitive.depthBias.biasConstantFactor;
    rasterizer.depthBiasClamp = options.primitive.depthBias.biasClamp;
    rasterizer.depthBiasSlopeFactor = options.primitive.depthBias.biasSlopeFactor;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = options.multisample.samples > SampleCountFlagBits::Samples1Bit;
    multisampling.rasterizationSamples = sampleCountFlagBitsToVkSampleFlagBits(options.multisample.samples);
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = options.multisample.sampleMasks.data();
    multisampling.alphaToCoverageEnable = options.multisample.alphaToCoverageEnabled;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing
    auto vkStencilOpStateFromStencilOperationOptions = [](const StencilOperationOptions &options) {
        VkStencilOpState stencilOp = {};
        stencilOp.failOp = stencilOperationToVkStencilOp(options.failOp);
        stencilOp.passOp = stencilOperationToVkStencilOp(options.passOp);
        stencilOp.depthFailOp = stencilOperationToVkStencilOp(options.depthFailOp);
        stencilOp.compareOp = compareOperationToVkCompareOp(options.compareOp);
        stencilOp.compareMask = options.compareMask;
        stencilOp.writeMask = options.writeMask;
        stencilOp.reference = options.reference;
        return stencilOp;
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = options.depthStencil.depthTestEnabled;
    depthStencil.depthWriteEnable = options.depthStencil.depthWritesEnabled;
    depthStencil.depthCompareOp = compareOperationToVkCompareOp(options.depthStencil.depthCompareOperation);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = options.depthStencil.stencilTestEnabled;
    depthStencil.front = vkStencilOpStateFromStencilOperationOptions(options.depthStencil.stencilFront);
    depthStencil.back = vkStencilOpStateFromStencilOperationOptions(options.depthStencil.stencilBack);

    // Blending
    const uint32_t attachmentCount = options.renderTargets.size();
    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlends;
    attachmentBlends.reserve(attachmentCount);

    std::vector<VkFormat> vkColorFormats;
    vkColorFormats.reserve(attachmentCount);

    for (uint32_t i = 0; i < attachmentCount; ++i) {
        const auto &renderTarget = options.renderTargets.at(i);

        VkPipelineColorBlendAttachmentState vkAttachmentBlend = {};
        vkAttachmentBlend.colorWriteMask = renderTarget.writeMask.toInt();
        vkAttachmentBlend.blendEnable = renderTarget.blending.blendingEnabled;
        vkAttachmentBlend.srcColorBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.color.srcFactor);
        vkAttachmentBlend.dstColorBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.color.dstFactor);
        vkAttachmentBlend.colorBlendOp = blendOperationToVkBlendOp(renderTarget.blending.color.operation);
        vkAttachmentBlend.srcAlphaBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.alpha.srcFactor);
        vkAttachmentBlend.dstAlphaBlendFactor = blendFactorToVkBlendFactor(renderTarget.blending.alpha.dstFactor);
        vkAttachmentBlend.alphaBlendOp = blendOperationToVkBlendOp(renderTarget.blending.alpha.operation);

        attachmentBlends.emplace_back(vkAttachmentBlend);
        vkColorFormats.emplace_back(formatToVkFormat(renderTarget.format));
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = static_cast<uint32_t>(attachmentBlends.size());
    colorBlending.pAttachments = attachmentBlends.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Dynamic pipeline state. This is state that can be overridden whilst recording
    // command buffers with commands such as vkCmdSetViewport or vkCmdSetScissor. We
    // always make the viewport and scissor states dynamic and require clients to
    // set these when recording.
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    for (auto dynamicState : options.dynamicState.enabledDynamicStates) {
        dynamicStates.push_back(dynamicStateToVkDynamicState(dynamicState));
    }

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();
    dynamicStateInfo.flags = 0;

    // We do still need to specify the number of viewports (and scissor rects) though
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr; // Provided by dynamic state
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr; // Provided by dynamic state

    // We will use VK_KHR_dynamic_rendering (core in Vulkan 1.3) if options.dynamicRendering is enabled
    // Otherwise we will resolve options.renderPass if provided or create an implicit render pass otherwise
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    Handle<RenderPass_t> vulkanRenderPassHandle = options.renderPass;

    if (!vulkanRenderPassHandle.isValid() && !options.dynamicRendering.enabled) {
        // Create a render pass that serves to specify the layout / compatibility of
        // concrete render passes and framebuffers used to perform rendering with this
        // pipeline at command record time. We only do this if the pipeline outputs to
        // render targets.
        // Specify attachment refs for all color and resolve render targets and any
        // depth-stencil target. Concrete render passes that want to use this pipeline
        // to render, must begin a render pass that is compatible with this render pass.
        // See the detailed description of render pass compatibility at:
        //
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#renderpass-compatibility
        //
        // But in short, the concrete render pass must match attachment counts of each
        // type and match the formats and sample counts in each case.

        vulkanRenderPassHandle = createImplicitRenderPass(deviceHandle,
                                                          options.renderTargets,
                                                          options.depthStencil,
                                                          options.multisample.samples,
                                                          options.viewCount);
    }

    // Bring it all together in the all-knowing pipeline create info
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStagesInfo.shaderInfos.size());
    pipelineInfo.pStages = shaderStagesInfo.shaderInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pTessellationState = &tessellationStateInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = vulkanPipelineLayout->pipelineLayout;

    VkBaseOutStructure *chainCurrent = reinterpret_cast<VkBaseOutStructure *>(&pipelineInfo);
    auto addToChain = [&chainCurrent](auto *next) {
        auto n = reinterpret_cast<VkBaseOutStructure *>(next);
        chainCurrent->pNext = n;
        chainCurrent = n;
    };

#if defined(VK_KHR_dynamic_rendering)
    VkPipelineRenderingCreateInfoKHR pipelineDynamicRenderingCreateInfo{};
    VkRenderingInputAttachmentIndexInfoKHR inputAttachmentLocations{};
    VkRenderingAttachmentLocationInfoKHR outputAttachmentLocations{};

    std::vector<uint32_t> outputLocations;
    std::vector<uint32_t> inputColorLocations;
    uint32_t inputDepthLocation{ VK_ATTACHMENT_UNUSED };
    uint32_t inputStencilLocation{ VK_ATTACHMENT_UNUSED };

    if (options.dynamicRendering.enabled) {
        assert(!vulkanRenderPassHandle.isValid()); // Dynamic Rendering is not compatible with explicit RenderPasses
        assert(vulkanDevice->requestedFeatures.dynamicRendering); // Dynamic Rendering feature should be enabled
        pipelineDynamicRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        pipelineDynamicRenderingCreateInfo.pNext = VK_NULL_HANDLE;

        const uint32_t multiViewMaskMask = uint32_t(1 << options.viewCount) - 1;
        pipelineDynamicRenderingCreateInfo.viewMask = (options.viewCount > 1) ? multiViewMaskMask : 0;

        pipelineDynamicRenderingCreateInfo.colorAttachmentCount = attachmentCount;
        pipelineDynamicRenderingCreateInfo.pColorAttachmentFormats = vkColorFormats.data();
        pipelineDynamicRenderingCreateInfo.depthAttachmentFormat = formatToVkFormat(options.depthStencil.format);
        pipelineDynamicRenderingCreateInfo.stencilAttachmentFormat = hasStencilFormat(options.depthStencil.format) ? formatToVkFormat(options.depthStencil.format) : VK_FORMAT_UNDEFINED;
        addToChain(&pipelineDynamicRenderingCreateInfo);

        inputAttachmentLocations.sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR;
        outputAttachmentLocations.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR;

        // Input Attachments Locations
        if (options.dynamicRendering.dynamicInputLocations) {
            inputDepthLocation = (options.dynamicRendering.dynamicInputLocations->inputDepthAttachment.enabled) ? options.dynamicRendering.dynamicInputLocations->inputDepthAttachment.remappedIndex : VK_ATTACHMENT_UNUSED;
            inputStencilLocation = (options.dynamicRendering.dynamicInputLocations->inputStencilAttachment.enabled) ? options.dynamicRendering.dynamicInputLocations->inputStencilAttachment.remappedIndex : VK_ATTACHMENT_UNUSED;

            inputAttachmentLocations.pDepthInputAttachmentIndex = (options.dynamicRendering.dynamicInputLocations->inputDepthAttachment.enabled) ? &inputDepthLocation : nullptr;
            inputAttachmentLocations.pStencilInputAttachmentIndex = (options.dynamicRendering.dynamicInputLocations->inputStencilAttachment.enabled) ? &inputStencilLocation : nullptr;

            inputColorLocations.reserve(options.dynamicRendering.dynamicInputLocations->inputColorAttachments.size());
            for (const DynamicAttachmentMapping &mapping : options.dynamicRendering.dynamicInputLocations->inputColorAttachments) {
                inputColorLocations.push_back(mapping.enabled ? mapping.remappedIndex : VK_ATTACHMENT_UNUSED);
            }
            inputAttachmentLocations.colorAttachmentCount = inputColorLocations.size();
            inputAttachmentLocations.pColorAttachmentInputIndices = inputColorLocations.data();
            addToChain(&inputAttachmentLocations);
        }

        // Output Attachments Locations
        if (options.dynamicRendering.dynamicOutputLocations) {
            outputLocations.reserve(options.dynamicRendering.dynamicOutputLocations->outputAttachments.size());
            for (const DynamicAttachmentMapping &mapping : options.dynamicRendering.dynamicOutputLocations->outputAttachments) {
                outputLocations.push_back(mapping.enabled ? mapping.remappedIndex : VK_ATTACHMENT_UNUSED);
            }
            outputAttachmentLocations.colorAttachmentCount = outputLocations.size();
            outputAttachmentLocations.pColorAttachmentLocations = outputLocations.data();
            addToChain(&outputAttachmentLocations);
        }
    } else
#endif
    {
        // Note: at the moment this render pass isn't shared. It might make sense to do so at some point,
        // in which case, the renderPass handle will have to be added to vulkanDevice->renderPasses
        VulkanRenderPass *vulkanRenderPass = m_renderPasses.get(vulkanRenderPassHandle);
        vkRenderPass = vulkanRenderPass->renderPass;
        pipelineInfo.renderPass = vkRenderPass;
        pipelineInfo.subpass = options.subpassIndex;
    }

    VkPipeline vkPipeline{ VK_NULL_HANDLE };
    if (auto result = vkCreateGraphicsPipelines(vulkanDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating graphics pipeline: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(vkPipeline), options.label);

    // Create VulkanPipeline object and return handle
    const auto vulkanGraphicsPipelineHandle = m_graphicsPipelines.emplace(VulkanGraphicsPipeline(
            vkPipeline,
            this,
            (options.renderPass.isValid() ? (Handle<KDGpu::RenderPass_t>()) : (vulkanRenderPassHandle)), // pipeline do not own the renderpass if it's passed in
            dynamicStates,
            deviceHandle,
            options.layout,
            options.dynamicRendering.enabled));

    return vulkanGraphicsPipelineHandle;
}

void VulkanResourceManager::deleteGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle)
{
    VulkanGraphicsPipeline *vulkanPipeline = m_graphicsPipelines.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanPipeline->deviceHandle);

    vkDestroyPipeline(vulkanDevice->device, vulkanPipeline->pipeline, nullptr);

    if (vulkanPipeline->renderPassHandle.isValid()) { // If the renderpass is not explicitly created by the user, we're in charge of releasing it
        VulkanRenderPass *vulkanRenderPass = m_renderPasses.get(vulkanPipeline->renderPassHandle);
        if (vulkanRenderPass) {
            vkDestroyRenderPass(vulkanDevice->device, vulkanRenderPass->renderPass, nullptr);
            m_renderPasses.remove(vulkanPipeline->renderPassHandle);
        }
    }

    m_graphicsPipelines.remove(handle);
}

VulkanGraphicsPipeline *VulkanResourceManager::getGraphicsPipeline(const Handle<GraphicsPipeline_t> &handle) const
{
    return m_graphicsPipelines.get(handle);
}

Handle<ComputePipeline_t> VulkanResourceManager::createComputePipeline(const Handle<Device_t> &deviceHandle, const ComputePipelineOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // Fetch the specified pipeline layout
    VulkanPipelineLayout *vulkanPipelineLayout = getPipelineLayout(options.layout);
    if (!vulkanPipelineLayout) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid pipeline layout requested");
        return {};
    }

    // Shader stages
    VkPipelineShaderStageCreateInfo computeShaderInfo{};
    computeShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

    VkSpecializationInfo shaderSpecializationInfo;
    std::vector<VkSpecializationMapEntry> shaderSpecializationMapEntries;
    std::vector<uint8_t> shaderSpecializationRawData;

    // Lookup the shader module
    const auto vulkanShaderModule = getShaderModule(options.shaderStage.shaderModule);
    if (!vulkanShaderModule)
        return {};
    computeShaderInfo.module = vulkanShaderModule->shaderModule;
    computeShaderInfo.pName = options.shaderStage.entryPoint.data();

    if (!options.shaderStage.specializationConstants.empty()) {
        uint32_t byteOffset = 0;
        const size_t specializationConstantsCount = options.shaderStage.specializationConstants.size();
        shaderSpecializationMapEntries.reserve(specializationConstantsCount);

        for (size_t sCI = 0; sCI < specializationConstantsCount; ++sCI) {
            const SpecializationConstant &specializationConstant = options.shaderStage.specializationConstants[sCI];
            const SpecializationConstantData &specializationConstantData = getByteOffsetSizeAndRawValueForSpecializationConstant(specializationConstant);

            shaderSpecializationMapEntries.emplace_back(VkSpecializationMapEntry{
                    .constantID = specializationConstant.constantId,
                    .offset = byteOffset,
                    .size = specializationConstantData.byteSize,
            });

            // Append Raw Byte Values
            const std::vector<uint8_t> &rawData = specializationConstantData.byteValues;
            shaderSpecializationRawData.insert(shaderSpecializationRawData.end(), rawData.begin(), rawData.end());

            // Increase offset
            byteOffset += specializationConstantData.byteSize;
        }

        shaderSpecializationInfo.mapEntryCount = specializationConstantsCount;
        shaderSpecializationInfo.pMapEntries = shaderSpecializationMapEntries.data();
        shaderSpecializationInfo.dataSize = shaderSpecializationRawData.size();
        shaderSpecializationInfo.pData = shaderSpecializationRawData.data();
        computeShaderInfo.pSpecializationInfo = &shaderSpecializationInfo;
    }

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = computeShaderInfo;
    pipelineInfo.layout = vulkanPipelineLayout->pipelineLayout;

    VkPipeline vkPipeline{ VK_NULL_HANDLE };

    if (auto result = vkCreateComputePipelines(vulkanDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating compute pipeline: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(vkPipeline), options.label);

    // Create VulkanPipeline object and return handle
    const auto vulkanComputePipelineHandle = m_computePipelines.emplace(VulkanComputePipeline(
            vkPipeline,
            this,
            deviceHandle,
            options.layout));

    return vulkanComputePipelineHandle;
}
void VulkanResourceManager::deleteComputePipeline(const Handle<ComputePipeline_t> &handle)
{
    VulkanComputePipeline *vulkanPipeline = m_computePipelines.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanPipeline->deviceHandle);

    vkDestroyPipeline(vulkanDevice->device, vulkanPipeline->pipeline, nullptr);

    m_computePipelines.remove(handle);
}

VulkanComputePipeline *VulkanResourceManager::getComputePipeline(const Handle<ComputePipeline_t> &handle) const
{
    return m_computePipelines.get(handle);
}

Handle<RayTracingPipeline_t> VulkanResourceManager::createRayTracingPipeline(const Handle<Device_t> &deviceHandle,
                                                                             const RayTracingPipelineOptions &options)
{
#if defined(VK_KHR_ray_tracing_pipeline)
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // Fetch the specified pipeline layout
    VulkanPipelineLayout *vulkanPipelineLayout = getPipelineLayout(options.layout);
    if (!vulkanPipelineLayout) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid pipeline layout requested");
        return {};
    }

    // Shader stages
    ShaderStagesInfo shaderStagesInfo;
    if (!fillShaderStageInfos(options.shaderStages, shaderStagesInfo)) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Failed to build shader stages info for Pipeline");
        return {};
    }

    // Shader groups
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupsInfo;
    shaderGroupsInfo.reserve(options.shaderGroups.size());

    for (const RayTracingShaderGroupOptions &group : options.shaderGroups) {
        VkRayTracingShaderGroupCreateInfoKHR groupInfo{};
        groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        groupInfo.type = rayTracingShaderGroupTypeToVkRayTracingShaderGroupType(group.type);
        groupInfo.generalShader = VK_SHADER_UNUSED_KHR;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;

        switch (group.type) {
        case RayTracingShaderGroupType::General: {
            groupInfo.generalShader = group.generalShaderIndex.value();
            break;
        };
        case RayTracingShaderGroupType::ProceduralHit: {
            groupInfo.intersectionShader = group.intersectionShaderIndex.value();
            groupInfo.anyHitShader = group.anyHitShaderIndex.has_value() ? group.anyHitShaderIndex.value() : VK_SHADER_UNUSED_KHR;
            groupInfo.closestHitShader = group.closestHitShaderIndex.has_value() ? group.closestHitShaderIndex.value() : VK_SHADER_UNUSED_KHR;
            break;
        };
        case RayTracingShaderGroupType::TrianglesHit: {
            groupInfo.anyHitShader = group.anyHitShaderIndex.has_value() ? group.anyHitShaderIndex.value() : VK_SHADER_UNUSED_KHR;
            groupInfo.closestHitShader = group.closestHitShaderIndex.has_value() ? group.closestHitShaderIndex.value() : VK_SHADER_UNUSED_KHR;
            break;
        };
        }

        shaderGroupsInfo.emplace_back(groupInfo);
    };

    // Dynamic pipeline state.
    std::vector<VkDynamicState> dynamicStates = {
        // VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();
    dynamicStateInfo.flags = 0;

    // MaxRecursionDepth
    uint32_t maxRecursionDepth = options.maxRecursionDepth;
    if (maxRecursionDepth == 0) {
        VulkanAdapter *vulkanAdapter = getAdapter(vulkanDevice->adapterHandle);
        maxRecursionDepth = vulkanAdapter->queryAdapterProperties().rayTracingProperties.maxRayRecursionDepth;
    }

    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStagesInfo.shaderInfos.size());
    pipelineInfo.pStages = shaderStagesInfo.shaderInfos.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroupsInfo.size());
    pipelineInfo.pGroups = shaderGroupsInfo.data();
    pipelineInfo.maxPipelineRayRecursionDepth = maxRecursionDepth;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = vulkanPipelineLayout->pipelineLayout;

    VkPipeline vkPipeline{ VK_NULL_HANDLE };

    assert(vulkanDevice->vkCreateRayTracingPipelinesKHR != nullptr);
    if (auto result = vulkanDevice->vkCreateRayTracingPipelinesKHR(vulkanDevice->device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating raytracing pipeline: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(vkPipeline), options.label);

    // Create VulkanPipeline object and return handle
    const auto vulkanRayTracingPipelineHandle = m_rayTracingPipelines.emplace(VulkanRayTracingPipeline(
            vkPipeline,
            this,
            deviceHandle,
            options.layout));

    return vulkanRayTracingPipelineHandle;
#else
    assert(false);
    return {};
#endif
}

void VulkanResourceManager::deleteRayTracingPipeline(const Handle<RayTracingPipeline_t> &handle)
{
    VulkanRayTracingPipeline *vulkanPipeline = m_rayTracingPipelines.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanPipeline->deviceHandle);

    vkDestroyPipeline(vulkanDevice->device, vulkanPipeline->pipeline, nullptr);

    m_rayTracingPipelines.remove(handle);
}

VulkanRayTracingPipeline *VulkanResourceManager::getRayTracingPipeline(const Handle<RayTracingPipeline_t> &handle) const
{
    return m_rayTracingPipelines.get(handle);
}

Handle<GpuSemaphore_t> VulkanResourceManager::createGpuSemaphore(const Handle<Device_t> &deviceHandle, const GpuSemaphoreOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkExportSemaphoreCreateInfo exportSemaphoreCreateInfo = {};
    if (options.externalSemaphoreHandleType != ExternalSemaphoreHandleTypeFlagBits::None) {
        exportSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
        exportSemaphoreCreateInfo.pNext = nullptr;
        exportSemaphoreCreateInfo.handleTypes = externalSemaphoreHandleTypeToVkExternalSemaphoreHandleType(options.externalSemaphoreHandleType);
        semaphoreInfo.pNext = &exportSemaphoreCreateInfo;
    }

    VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
    if (auto result = vkCreateSemaphore(vulkanDevice->device, &semaphoreInfo, nullptr, &vkSemaphore); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating semaphore: {}", result);
        return {};
    }

    HandleOrFD externalSemaphoreHandle{};
    if (options.externalSemaphoreHandleType == ExternalSemaphoreHandleTypeFlagBits::OpaqueFD) {
#if defined(VK_KHR_external_semaphore_fd)
        if (vulkanDevice->vkGetSemaphoreFdKHR) {
            VkSemaphoreGetFdInfoKHR vulkanSemaphoreGetFdInfoKHR = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR,
                .pNext = nullptr,
                .semaphore = vkSemaphore,
                .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR
            };
            int fd{};
            vulkanDevice->vkGetSemaphoreFdKHR(vulkanDevice->device, &vulkanSemaphoreGetFdInfoKHR, &fd);
            externalSemaphoreHandle = fd;
        }
#else
        assert(false);
#endif
    } else if (options.externalSemaphoreHandleType == ExternalSemaphoreHandleTypeFlagBits::OpaqueWin32) {
#if defined(VK_KHR_external_fence_win32)
        if (vulkanDevice->vkGetSemaphoreWin32HandleKHR) {
            VkSemaphoreGetWin32HandleInfoKHR vulkanSemaphoreGetHandleInfoKHR = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR,
                .pNext = nullptr,
                .semaphore = vkSemaphore,
                .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT
            };
            HANDLE winHandle{};
            vulkanDevice->vkGetSemaphoreWin32HandleKHR(vulkanDevice->device, &vulkanSemaphoreGetHandleInfoKHR, &winHandle);
            externalSemaphoreHandle = winHandle;
        }
#else
        assert(false);
#endif
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_SEMAPHORE, reinterpret_cast<uint64_t>(vkSemaphore), options.label);

    const auto vulkanGpuSemaphoreHandle = m_gpuSemaphores.emplace(VulkanGpuSemaphore(
            vkSemaphore,
            this,
            deviceHandle,
            externalSemaphoreHandle));

    return vulkanGpuSemaphoreHandle;
}

void VulkanResourceManager::deleteGpuSemaphore(const Handle<GpuSemaphore_t> &handle)
{
    VulkanGpuSemaphore *vulkanSemaphore = m_gpuSemaphores.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanSemaphore->deviceHandle);

    vkDestroySemaphore(vulkanDevice->device, vulkanSemaphore->semaphore, nullptr);

    m_gpuSemaphores.remove(handle);
}

VulkanGpuSemaphore *VulkanResourceManager::getGpuSemaphore(const Handle<GpuSemaphore_t> &handle) const
{
    return m_gpuSemaphores.get(handle);
}

Handle<CommandRecorder_t> VulkanResourceManager::createCommandRecorder(const Handle<Device_t> &deviceHandle, const CommandRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // Which queue is the command recorder requested for?

    QueueDescription *queueDescription = nullptr;

    if (!options.queue.isValid()) {
        if (vulkanDevice->queueDescriptions.empty()) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "No more queue descriptors available for device");
            return {};
        }
        queueDescription = &vulkanDevice->queueDescriptions[0];
    } else {
        // Look for this queue on the device
        const auto it = std::find_if(
                vulkanDevice->queueDescriptions.begin(),
                vulkanDevice->queueDescriptions.end(),
                [options](const QueueDescription &queueDescription) { return queueDescription.queue == options.queue; });
        if (it == vulkanDevice->queueDescriptions.end()) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "Cannot find requested queue for device");
            return {};
        }
        queueDescription = &(*it);
    }

    Handle<Queue_t> queueHandle = queueDescription->queue;
    const uint32_t queueTypeIndex = queueDescription->queueTypeIndex;
    assert(queueHandle.isValid());
    assert(queueTypeIndex != std::numeric_limits<uint32_t>::max());

    // Find or create a command pool for this combination of thread and queue family
    if (vulkanDevice->commandPools[queueTypeIndex] == VK_NULL_HANDLE) {
        // No command pool exists yet for this queue family, let's create one why not!
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueTypeIndex;

        VkCommandPool vkCommandPool = VK_NULL_HANDLE;
        if (auto result = vkCreateCommandPool(vulkanDevice->device, &poolInfo, nullptr, &vkCommandPool); result != VK_SUCCESS) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating command pool for queue family {}: {}", queueTypeIndex, result);
            return {};
        }
        vulkanDevice->commandPools[queueTypeIndex] = vkCommandPool;
    }

    // Create the Command Buffer
    VkCommandPool vkCommandPool = vulkanDevice->commandPools[queueTypeIndex];
    const Handle<CommandBuffer_t> commandBufferHandle = createCommandBuffer(deviceHandle,
                                                                            *queueDescription,
                                                                            options.level);

    // Finally, we can create the command recorder object
    const auto vulkanCommandRecorderHandle = m_commandRecorders.emplace(VulkanCommandRecorder(
            vkCommandPool,
            commandBufferHandle,
            this,
            deviceHandle));

    return vulkanCommandRecorderHandle;
}

void VulkanResourceManager::deleteCommandRecorder(const Handle<CommandRecorder_t> &handle)
{
    // VulkanCommandRecorder actually doesn't map to an actual Vulkan Resource.
    // It creates a VulkanCommandBuffer that holds VkCommandBuffer and whose lifetime
    m_commandRecorders.remove(handle);
}

VulkanCommandRecorder *VulkanResourceManager::getCommandRecorder(const Handle<CommandRecorder_t> &handle) const
{
    return m_commandRecorders.get(handle);
}

Handle<CommandBuffer_t> VulkanResourceManager::createCommandBuffer(const Handle<Device_t> &deviceHandle,
                                                                   const QueueDescription &queueDescription,
                                                                   CommandBufferLevel commandLevel)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);
    VkCommandPool vkCommandPool = vulkanDevice->commandPools[queueDescription.queueTypeIndex];

    // Allocate a command buffer object from the pool
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkCommandPool;
    allocInfo.level = commandBufferLevelToVkCommandBufferLevel(commandLevel);
    allocInfo.commandBufferCount = 1U;

    VkCommandBuffer vkCommandBuffer{ VK_NULL_HANDLE };
    if (auto result = vkAllocateCommandBuffers(vulkanDevice->device, &allocInfo, &vkCommandBuffer); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating allocating command buffers: {}", result);
        return {};
    }

    const auto vulkanCommandBufferHandle = m_commandBuffers.emplace(VulkanCommandBuffer(vkCommandBuffer,
                                                                                        vkCommandPool,
                                                                                        allocInfo.level,
                                                                                        this,
                                                                                        deviceHandle));

    return vulkanCommandBufferHandle;
}

void VulkanResourceManager::deleteCommandBuffer(const Handle<CommandBuffer_t> &handle)
{
    VulkanCommandBuffer *commandBuffer = m_commandBuffers.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(commandBuffer->deviceHandle);

    for (const Handle<Buffer_t> buf : commandBuffer->temporaryBuffersToRelease)
        deleteBuffer(buf);

    vkFreeCommandBuffers(vulkanDevice->device, commandBuffer->commandPool, 1, &commandBuffer->commandBuffer);
    m_commandBuffers.remove(handle);
}

VulkanCommandBuffer *VulkanResourceManager::getCommandBuffer(const Handle<CommandBuffer_t> &handle) const
{
    return m_commandBuffers.get(handle);
}

Handle<BindGroupPool_t> VulkanResourceManager::createBindGroupPool(const Handle<Device_t> &deviceHandle, const BindGroupPoolOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(9);
    if (options.uniformBufferCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, options.uniformBufferCount });
    if (options.dynamicUniformBufferCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, options.dynamicUniformBufferCount });
    if (options.storageBufferCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, options.storageBufferCount });
    if (options.textureSamplerCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, options.textureSamplerCount });
    if (options.textureCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, options.textureCount });
    if (options.samplerCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER, options.samplerCount });
    if (options.imageCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, options.imageCount });
    if (options.inputAttachmentCount > 0)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, options.inputAttachmentCount });
    if (options.accelerationStructureCount > 0 && vulkanDevice->requestedFeatures.accelerationStructures)
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, options.accelerationStructureCount });

    VkDescriptorPool pool{ VK_NULL_HANDLE };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = options.maxBindGroupCount;
    poolInfo.flags = bindGroupPoolFlagsToVkDescriptorPoolCreateFlags(options.flags);

    if (auto result = vkCreateDescriptorPool(vulkanDevice->device, &poolInfo, nullptr, &pool); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating bindgroup pool: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_DESCRIPTOR_POOL, reinterpret_cast<uint64_t>(pool), options.label);

    const auto vulkanBindGroupPoolHandle = m_bindGroupPools.emplace(VulkanBindGroupPool(pool, this, deviceHandle, options.maxBindGroupCount, options.flags));
    return vulkanBindGroupPoolHandle;
}

void VulkanResourceManager::deleteBindGroupPool(const Handle<BindGroupPool_t> &handle)
{
    VulkanBindGroupPool *bindGroupPool = m_bindGroupPools.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(bindGroupPool->deviceHandle);

    vkDestroyDescriptorPool(vulkanDevice->device, bindGroupPool->descriptorPool, nullptr);

    // Reset descriptorSet handle of all the bind groups that were created from this pool in case they have no implicit free
    const auto referencedBindGroups = bindGroupPool->bindGroups();
    for (const Handle<BindGroup_t> &bindGroupHandle : referencedBindGroups) {
        VulkanBindGroup *vulkanBindGroup = m_bindGroups.get(bindGroupHandle);
        if (vulkanBindGroup != nullptr) {
            // If the bind group is still valid, we can delete it
            vulkanBindGroup->descriptorSet = VK_NULL_HANDLE;
        }
    }

    m_bindGroupPools.remove(handle);
}

VulkanBindGroupPool *VulkanResourceManager::getBindGroupPool(const Handle<BindGroupPool_t> &handle) const
{
    return m_bindGroupPools.get(handle);
}

Handle<RenderPassCommandRecorder_t> VulkanResourceManager::createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                           const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                           const RenderPassCommandRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    // Find or create a render pass object that matches the request
    const VulkanRenderPassKey renderPassKey(options, this);
    auto itRenderPass = vulkanDevice->renderPasses.find(renderPassKey);
    Handle<RenderPass_t> vulkanRenderPassHandle{};

    if (itRenderPass == vulkanDevice->renderPasses.end()) {
        // Create the render pass and cache the handle for it
        vulkanRenderPassHandle = createImplicitRenderPass(deviceHandle,
                                                          options.colorAttachments,
                                                          options.depthStencilAttachment,
                                                          options.samples,
                                                          options.viewCount);
        vulkanDevice->renderPasses.insert({ renderPassKey, vulkanRenderPassHandle });
    } else {
        vulkanRenderPassHandle = itRenderPass->second;
    }

    // Create Attachments from the ColorAttachments and DepthAttachment
    std::vector<Attachment> attachments;
    attachments.reserve(options.colorAttachments.size() + 1);

    for (const ColorAttachment &colorAttachment : options.colorAttachments) {
        attachments.emplace_back(Attachment{
                .view = colorAttachment.view,
                .resolveView = colorAttachment.resolveView,
                .color = Attachment::ColorOperations{
                        .clearValue = colorAttachment.clearValue,
                        .layout = colorAttachment.layout,
                },
        });
    }

    if (options.depthStencilAttachment.view.isValid()) {
        attachments.emplace_back(Attachment{
                .view = options.depthStencilAttachment.view,
                .resolveView = options.depthStencilAttachment.resolveView,
                .depth = Attachment::DepthStencilOperations{
                        .clearValue = DepthStencilClearValue{
                                .depthClearValue = options.depthStencilAttachment.depthClearValue,
                                .stencilClearValue = options.depthStencilAttachment.stencilClearValue },
                        .layout = options.depthStencilAttachment.layout,
                },
        });
    }

    return createRenderPassCommandRecorder(deviceHandle, commandRecorderHandle, RenderPassCommandRecorderWithRenderPassOptions{
                                                                                        .renderPass = vulkanRenderPassHandle,
                                                                                        .attachments = std::move(attachments),
                                                                                        .samples = options.samples,
                                                                                        .viewCount = options.viewCount,
                                                                                        .framebufferWidth = options.framebufferWidth,
                                                                                        .framebufferHeight = options.framebufferHeight,
                                                                                        .framebufferArrayLayers = options.framebufferArrayLayers,
                                                                                });
}

Handle<RenderPassCommandRecorder_t> VulkanResourceManager::createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                           const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                           const RenderPassCommandRecorderWithRenderPassOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);
    Handle<RenderPass_t> vulkanRenderPassHandle = options.renderPass;

    assert(vulkanRenderPassHandle.isValid());
    VulkanRenderPass *vulkanRenderPass = m_renderPasses.get(vulkanRenderPassHandle);
    if (!vulkanRenderPass) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Unable to find render pass");
        return {};
    }
    VkRenderPass vkRenderPass = vulkanRenderPass->renderPass;

    const bool usingMsaa = options.samples > SampleCountFlagBits::Samples1Bit;

    // Find or create a framebuffer as per the render pass above
    VulkanAttachmentKey attachmentKey;
    for (const auto &attachment : options.attachments) {
        // verify that only color or depth is defined
        if (attachment.color.has_value() == attachment.depth.has_value()) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "Both or none of the color and depth operations are defined!"
                                                  "color has value is {} while depth has value is {}",
                                attachment.color.has_value(), attachment.depth.has_value());
            return {};
        }
        attachmentKey.addAttachmentView(attachment.view);
        // Include resolve attachments if using MSAA.
        if (usingMsaa && attachment.resolveView.isValid())
            attachmentKey.addAttachmentView(attachment.resolveView);
    }

    uint32_t fbWidth = options.framebufferWidth;
    uint32_t fbHeight = options.framebufferHeight;
    uint32_t fbArrayLayers = options.framebufferArrayLayers;

    const bool shouldFetchTexture = (fbWidth == 0 || fbHeight == 0) || fbArrayLayers == 0;
    if (shouldFetchTexture) {
        const auto attachmentFirstColorAttachment = std::find_if(options.attachments.begin(), options.attachments.end(),
                                                                 [](const Attachment &x) { return x.color.has_value(); });
        if (attachmentFirstColorAttachment != options.attachments.end()) {
            VulkanTextureView *firstView = getTextureView(attachmentFirstColorAttachment->view);
            if (!firstView) {
                SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid texture view when creating render pass");
                return {};
            }

            VulkanTexture *firstTexture = getTexture(firstView->textureHandle);
            if (!firstTexture) {
                SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid texture when creating render pass");
                return {};
            }
            if ((fbWidth == 0 || fbHeight == 0)) {
                // Take the dimensions of the first attachment as the framebuffer dimensions
                fbWidth = firstTexture->extent.width;
                fbHeight = firstTexture->extent.height;
            }

            if (fbArrayLayers == 0) {
                fbArrayLayers = firstTexture->arrayLayers;
            }
        }
    }

    // TODO: Use VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT to create just one framebuffer rather than
    // 1 per swapchain image by allowing us to defer the image configuration to the begin render
    // pass call. See.
    // https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/extensions/VK_KHR_imageless_framebuffer.adoc
    VulkanFramebufferKey framebufferKey;
    framebufferKey.renderPass = vulkanRenderPassHandle;
    framebufferKey.attachmentsKey = attachmentKey;
    framebufferKey.width = fbWidth;
    framebufferKey.height = fbHeight;
    framebufferKey.layers = fbArrayLayers;
    framebufferKey.viewCount = options.viewCount;

    if (options.viewCount > 1)
        framebufferKey.layers = 1;

    auto itFramebuffer = vulkanDevice->framebuffers.find(framebufferKey);
    Handle<Framebuffer_t> vulkanFramebufferHandle;
    if (itFramebuffer == vulkanDevice->framebuffers.end()) {
        // Create the framebuffer and cache the handle for it
        vulkanFramebufferHandle = createFramebuffer(deviceHandle, framebufferKey);
        vulkanDevice->framebuffers.insert({ framebufferKey, vulkanFramebufferHandle });
    } else {
        vulkanFramebufferHandle = itFramebuffer->second;
    }

    VulkanFramebuffer *vulkanFramebuffer = m_framebuffers.get(vulkanFramebufferHandle);
    if (!vulkanFramebuffer) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not create or find a framebuffer");
        return {};
    }

    VkFramebuffer vkFramebuffer = vulkanFramebuffer->framebuffer;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = vkFramebuffer;

    // Render area - assume full view area for now. Can expose as an option later if needed.
    renderPassInfo.renderArea = {
        .offset = { .x = 0, .y = 0 },
        .extent = { .width = fbWidth, .height = fbHeight }
    };

    // Clear values - at most 2 x color attachments + depth
    // Maybe update in the future since a complex renderpass with subpass could exceed that? maybe 50?
    constexpr size_t MaxAttachmentCount = 20;
    assert(2 * options.attachments.size() + 1 <= MaxAttachmentCount);
    std::array<VkClearValue, MaxAttachmentCount> vkClearValues;
    size_t clearIdx = 0;

    const bool isAnyOfTheAttachmentsToBeCleared = std::ranges::any_of(vulkanRenderPass->attachmentDescriptions.begin(),
                                                                      vulkanRenderPass->attachmentDescriptions.end(),
                                                                      [](const AttachmentDescription &attDescription) {
                                                                          return attDescription.loadOperation == AttachmentLoadOperation::Clear || attDescription.stencilLoadOperation == AttachmentLoadOperation::Clear;
                                                                      });

    for (const auto &attachment : options.attachments) {
        if (attachment.color.has_value()) {
            auto arg = attachment.color.value();
            VkClearValue vkClearValue = {};
            vkClearValue.color.uint32[0] = arg.clearValue.uint32[0];
            vkClearValue.color.uint32[1] = arg.clearValue.uint32[1];
            vkClearValue.color.uint32[2] = arg.clearValue.uint32[2];
            vkClearValue.color.uint32[3] = arg.clearValue.uint32[3];
            vkClearValues[clearIdx++] = vkClearValue;

            // Include clear color again if using a resolve view. Must match number of attachments.
            if (attachment.resolveView.isValid())
                vkClearValues[clearIdx++] = vkClearValue;
        } else if (attachment.depth.has_value()) {
            auto arg = attachment.depth.value();
            VkClearValue vkClearValue = {};
            vkClearValue.depthStencil.depth = arg.clearValue.depthClearValue;
            vkClearValue.depthStencil.stencil = arg.clearValue.stencilClearValue;
            vkClearValues[clearIdx++] = vkClearValue;

            // Include depth clear again if using a Depth Resolve view. Must match number of attachments.
            if (attachment.resolveView.isValid()) {
                vkClearValues[clearIdx++] = vkClearValue;
                attachmentKey.addAttachmentView(attachment.resolveView);
            }
        }
    }

    const size_t expectedAttachmentCount = clearIdx; // options.attachments.size() + 1 for each attachment that has a resolveView if msaa is enabled
    if (expectedAttachmentCount != vulkanRenderPass->attachmentDescriptions.size()) {
        SPDLOG_LOGGER_WARN(Logger::logger(), "Mismatch between RenderPass expected attachments {} and provided attachments {}", vulkanRenderPass->attachmentDescriptions.size(), clearIdx);
    }

    renderPassInfo.clearValueCount = isAnyOfTheAttachmentsToBeCleared ? clearIdx : 0;
    renderPassInfo.pClearValues = vkClearValues.data();

    VulkanCommandRecorder *vulkanCommandRecorder = m_commandRecorders.get(commandRecorderHandle);
    if (!vulkanCommandRecorder) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid command recorder");
        return {};
    }
    VkCommandBuffer vkCommandBuffer = vulkanCommandRecorder->commandBuffer;

    vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    const auto vulkanRenderPassCommandRecorderHandle = m_renderPassCommandRecorders.emplace(
            VulkanRenderPassCommandRecorder(vkCommandBuffer, renderPassInfo.renderArea, this, deviceHandle, false));
    return vulkanRenderPassCommandRecorderHandle;
}

Handle<RenderPassCommandRecorder_t> VulkanResourceManager::createRenderPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                           const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                           const RenderPassCommandRecorderWithDynamicRenderingOptions &options)
{
#if defined(VK_KHR_dynamic_rendering)
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    uint32_t fbWidth = options.framebufferWidth;
    uint32_t fbHeight = options.framebufferHeight;
    uint32_t fbArrayLayers = options.framebufferArrayLayers;

    bool shouldFetchTextureToDetermineFBDimensions = (fbWidth == 0 || fbHeight == 0) || fbArrayLayers == 0;

    // Fill Color Attachments Info
    std::vector<VkRenderingAttachmentInfoKHR> colorAttachmentsInfo;
    colorAttachmentsInfo.reserve(options.colorAttachments.size());
    for (const ColorAttachment &attachment : options.colorAttachments) {
        VulkanTextureView *view = getTextureView(attachment.view);
        if (!view) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid texture view for Attachment");
            return {};
        }

        // If no FB dimensions set, determine these by retrieving the texture attached to the view
        if (shouldFetchTextureToDetermineFBDimensions) {
            VulkanTexture *texture = getTexture(view->textureHandle);
            if (!texture) {
                SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid texture for Attachment");
                return {};
            }

            if ((fbWidth == 0 || fbHeight == 0)) {
                // Take the dimensions of the first attachment as the framebuffer dimensions
                fbWidth = texture->extent.width;
                fbHeight = texture->extent.height;
            }

            if (fbArrayLayers == 0) {
                fbArrayLayers = texture->arrayLayers;
            }

            shouldFetchTextureToDetermineFBDimensions = false;
        }

        VulkanTextureView *resolveView = nullptr;
        if (attachment.resolveView.isValid()) {
            resolveView = getTextureView(attachment.resolveView);
            if (!resolveView) {
                SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid resolve texture view for Attachment");
                return {};
            }
        }

        VkClearValue vkClearValue = {};
        vkClearValue.color.uint32[0] = attachment.clearValue.uint32[0];
        vkClearValue.color.uint32[1] = attachment.clearValue.uint32[1];
        vkClearValue.color.uint32[2] = attachment.clearValue.uint32[2];
        vkClearValue.color.uint32[3] = attachment.clearValue.uint32[3];

        colorAttachmentsInfo.emplace_back(VkRenderingAttachmentInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .pNext = nullptr,
                .imageView = view->imageView,
                .imageLayout = textureLayoutToVkImageLayout(attachment.layout),
                .resolveMode = (resolveView) ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE,
                .resolveImageView = (resolveView) ? resolveView->imageView : VK_NULL_HANDLE,
                .resolveImageLayout = textureLayoutToVkImageLayout(attachment.layout),
                .loadOp = attachmentLoadOperationToVkAttachmentLoadOp(attachment.loadOperation),
                .storeOp = attachmentStoreOperationToVkAttachmentStoreOp(attachment.storeOperation),
                .clearValue = vkClearValue,
        });
    }

    // Fill Depth & Stencil Attachments Info
    VkRenderingAttachmentInfoKHR depthAttachmentsInfo{};
    VkRenderingAttachmentInfoKHR stencilAttachmentsInfo{};
    const bool hasDepthAttachment = options.depthStencilAttachment.view.isValid();
    const bool hasStencilAttachment = hasDepthAttachment;

    if (hasDepthAttachment) {
        depthAttachmentsInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        stencilAttachmentsInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;

        VulkanTextureView *view = getTextureView(options.depthStencilAttachment.view);
        if (!view) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid texture view for Depth/Stencil Attachment");
            return {};
        }

        VulkanTextureView *resolveView = nullptr;
        if (options.depthStencilAttachment.resolveView.isValid()) {
            resolveView = getTextureView(options.depthStencilAttachment.resolveView);
            if (!resolveView) {
                SPDLOG_LOGGER_ERROR(Logger::logger(), "Invalid resolve texture view Depth/Stencil Attachment");
                return {};
            }
        }

        depthAttachmentsInfo.imageView = view->imageView;
        depthAttachmentsInfo.imageLayout = textureLayoutToVkImageLayout(options.depthStencilAttachment.layout);
        depthAttachmentsInfo.resolveMode = (resolveView) ? resolveModeToVkResolveMode(options.depthStencilAttachment.depthResolveMode) : VK_RESOLVE_MODE_NONE;
        depthAttachmentsInfo.resolveImageView = (resolveView) ? resolveView->imageView : VK_NULL_HANDLE;
        depthAttachmentsInfo.resolveImageLayout = textureLayoutToVkImageLayout(options.depthStencilAttachment.layout);
        depthAttachmentsInfo.loadOp = attachmentLoadOperationToVkAttachmentLoadOp(options.depthStencilAttachment.depthLoadOperation);
        depthAttachmentsInfo.storeOp = attachmentStoreOperationToVkAttachmentStoreOp(options.depthStencilAttachment.depthStoreOperation);
        depthAttachmentsInfo.clearValue.depthStencil.depth = options.depthStencilAttachment.depthClearValue;

        stencilAttachmentsInfo.imageView = view->imageView;
        stencilAttachmentsInfo.imageLayout = textureLayoutToVkImageLayout(options.depthStencilAttachment.layout),
        stencilAttachmentsInfo.resolveMode = (resolveView) ? resolveModeToVkResolveMode(options.depthStencilAttachment.stencilResolveMode) : VK_RESOLVE_MODE_NONE;
        stencilAttachmentsInfo.resolveImageView = (resolveView) ? resolveView->imageView : VK_NULL_HANDLE;
        stencilAttachmentsInfo.resolveImageLayout = textureLayoutToVkImageLayout(options.depthStencilAttachment.layout);
        stencilAttachmentsInfo.loadOp = attachmentLoadOperationToVkAttachmentLoadOp(options.depthStencilAttachment.stencilLoadOperation);
        stencilAttachmentsInfo.storeOp = attachmentStoreOperationToVkAttachmentStoreOp(options.depthStencilAttachment.stencilStoreOperation);
        stencilAttachmentsInfo.clearValue.depthStencil.stencil = options.depthStencilAttachment.stencilClearValue;
    }

    // Retrieve Command Buffer
    VulkanCommandRecorder *vulkanCommandRecorder = m_commandRecorders.get(commandRecorderHandle);
    if (!vulkanCommandRecorder) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid command recorder");
        return {};
    }
    VkCommandBuffer vkCommandBuffer = vulkanCommandRecorder->commandBuffer;

    // Fill up dynamic rendering struct
    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;

    // Render area - assume full view area for now. Can expose as an option later if needed.
    renderingInfo.renderArea = {
        .offset = { .x = 0, .y = 0 },
        .extent = { .width = fbWidth, .height = fbHeight }
    };
    renderingInfo.layerCount = fbArrayLayers;
    renderingInfo.layerCount = fbArrayLayers;

    const uint32_t multiViewMaskMask = uint32_t(1 << options.viewCount) - 1;
    renderingInfo.viewMask = (options.viewCount > 1) ? multiViewMaskMask : 0;

    renderingInfo.colorAttachmentCount = colorAttachmentsInfo.size();
    renderingInfo.pColorAttachments = colorAttachmentsInfo.data();
    renderingInfo.pDepthAttachment = hasDepthAttachment ? &depthAttachmentsInfo : nullptr;
    renderingInfo.pStencilAttachment = hasStencilAttachment ? &stencilAttachmentsInfo : nullptr;

    vulkanDevice->vkCmdBeginRenderingKHR(vkCommandBuffer, &renderingInfo);

    const auto vulkanRenderPassCommandRecorderHandle = m_renderPassCommandRecorders.emplace(
            VulkanRenderPassCommandRecorder(vkCommandBuffer, renderingInfo.renderArea, this, deviceHandle, true));
    return vulkanRenderPassCommandRecorderHandle;
#else
    SPDLOG_LOGGER_ERROR(Logger::logger(), "Dynamic Rendering not supported by this Vulkan SDK");
    return {};
#endif
}

VulkanRenderPassCommandRecorder *VulkanResourceManager::getRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle) const
{
    return m_renderPassCommandRecorders.get(handle);
}

void VulkanResourceManager::deleteRenderPassCommandRecorder(const Handle<RenderPassCommandRecorder_t> &handle)
{
    VulkanRenderPassCommandRecorder *vulkanCommandPassRecorder = m_renderPassCommandRecorders.get(handle);

    m_renderPassCommandRecorders.remove(handle);
}

Handle<ComputePassCommandRecorder_t> VulkanResourceManager::createComputePassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                             const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                             const ComputePassCommandRecorderOptions &)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VulkanCommandRecorder *vulkanCommandRecorder = m_commandRecorders.get(commandRecorderHandle);
    if (!vulkanCommandRecorder) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid command recorder");
        return {};
    }
    VkCommandBuffer vkCommandBuffer = vulkanCommandRecorder->commandBuffer;

    const auto vulkanComputePassCommandRecorderHandle = m_computePassCommandRecorders.emplace(
            VulkanComputePassCommandRecorder(vkCommandBuffer, this, deviceHandle));
    return vulkanComputePassCommandRecorderHandle;
}

void VulkanResourceManager::deleteComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle)
{
    VulkanComputePassCommandRecorder *vulkanCommandPassRecorder = m_computePassCommandRecorders.get(handle);

    m_computePassCommandRecorders.remove(handle);
}

VulkanComputePassCommandRecorder *VulkanResourceManager::getComputePassCommandRecorder(const Handle<ComputePassCommandRecorder_t> &handle) const
{
    return m_computePassCommandRecorders.get(handle);
}

void VulkanResourceManager::deleteRayTracingPassCommandRecorder(const Handle<RayTracingPassCommandRecorder_t> &handle)
{
    VulkanRayTracingPassCommandRecorder *vulkanCommandPassRecorder = m_rayTracingPassCommandRecorders.get(handle);

    m_rayTracingPassCommandRecorders.remove(handle);
}

VulkanRayTracingPassCommandRecorder *VulkanResourceManager::getRayTracingPassCommandRecorder(const Handle<RayTracingPassCommandRecorder_t> &handle) const
{
    return m_rayTracingPassCommandRecorders.get(handle);
}

Handle<RayTracingPassCommandRecorder_t> VulkanResourceManager::createRayTracingPassCommandRecorder(const Handle<Device_t> &deviceHandle,
                                                                                                   const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                                   const RayTracingPassCommandRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VulkanCommandRecorder *vulkanCommandRecorder = m_commandRecorders.get(commandRecorderHandle);
    if (!vulkanCommandRecorder) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid command recorder");
        return {};
    }
    VkCommandBuffer vkCommandBuffer = vulkanCommandRecorder->commandBuffer;

    const auto vulkanRayTracingPassCommandRecorderHandle = m_rayTracingPassCommandRecorders.emplace(
            VulkanRayTracingPassCommandRecorder(vkCommandBuffer, this, deviceHandle));
    return vulkanRayTracingPassCommandRecorderHandle;
}

// Called to create implicit RenderPass for RenderPassCommandRecorder
SubpassDescription VulkanResourceManager::fillAttachmentDescriptionAndCreateSubpassDescription(std::vector<AttachmentDescription> &attachmentDescriptions,
                                                                                               const std::vector<ColorAttachment> &colorAttachments,
                                                                                               const DepthStencilAttachment &depthAttachment,
                                                                                               SampleCountFlagBits samples)
{
    const bool useMultiSampling = samples > SampleCountFlagBits::Samples1Bit;

    SubpassDescription subpass;
    uint32_t currentAttachmentDescriptionIndex = 0;

    for (const ColorAttachment &attachment : colorAttachments) {
        AttachmentDescription &attachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
        attachmentDescription.format = formatFromTextureView(attachment.view);
        attachmentDescription.samples = samples;

        attachmentDescription.loadOperation = attachment.loadOperation;
        attachmentDescription.storeOperation = attachment.storeOperation;
        attachmentDescription.stencilLoadOperation = AttachmentLoadOperation::DontCare;
        attachmentDescription.stencilStoreOperation = AttachmentStoreOperation::DontCare;
        attachmentDescription.initialLayout = attachment.initialLayout;
        attachmentDescription.finalLayout = attachment.finalLayout;
        subpass.colorAttachmentReference.push_back({ currentAttachmentDescriptionIndex++, attachment.layout });

        if (useMultiSampling && attachment.resolveView.isValid()) {
            AttachmentDescription &resolveAttachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
            resolveAttachmentDescription.format = formatFromTextureView(attachment.resolveView);
            resolveAttachmentDescription.samples = SampleCountFlagBits::Samples1Bit;

            resolveAttachmentDescription.loadOperation = attachment.loadOperation;
            resolveAttachmentDescription.storeOperation = attachment.storeOperation;
            resolveAttachmentDescription.stencilLoadOperation = AttachmentLoadOperation::DontCare;
            resolveAttachmentDescription.stencilStoreOperation = AttachmentStoreOperation::DontCare;
            resolveAttachmentDescription.initialLayout = attachment.initialLayout;
            resolveAttachmentDescription.finalLayout = attachment.finalLayout;
            subpass.resolveAttachmentReference.push_back({ currentAttachmentDescriptionIndex++, attachment.layout });
        }
    }

    if (depthAttachment.view.isValid()) {
        AttachmentDescription &attachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
        attachmentDescription.format = formatFromTextureView(depthAttachment.view);
        attachmentDescription.samples = samples;

        attachmentDescription.loadOperation = depthAttachment.depthLoadOperation;
        attachmentDescription.storeOperation = depthAttachment.depthStoreOperation;
        attachmentDescription.stencilLoadOperation = depthAttachment.stencilLoadOperation;
        attachmentDescription.stencilStoreOperation = depthAttachment.stencilStoreOperation;
        attachmentDescription.initialLayout = depthAttachment.initialLayout;
        attachmentDescription.finalLayout = depthAttachment.finalLayout;
        if (subpass.depthAttachmentReference) {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "More than 1 Depth Attachment in this subpass!");
        }
        subpass.depthAttachmentReference = { currentAttachmentDescriptionIndex++, depthAttachment.layout };

        if (useMultiSampling && depthAttachment.resolveView.isValid()) {
            AttachmentDescription &resolveAttachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
            resolveAttachmentDescription.format = formatFromTextureView(depthAttachment.resolveView);
            resolveAttachmentDescription.samples = SampleCountFlagBits::Samples1Bit;

            resolveAttachmentDescription.loadOperation = depthAttachment.depthLoadOperation;
            resolveAttachmentDescription.storeOperation = depthAttachment.depthStoreOperation;
            resolveAttachmentDescription.stencilLoadOperation = depthAttachment.stencilLoadOperation;
            resolveAttachmentDescription.stencilStoreOperation = depthAttachment.stencilStoreOperation;
            resolveAttachmentDescription.initialLayout = depthAttachment.initialLayout;
            resolveAttachmentDescription.finalLayout = depthAttachment.finalLayout;
            if (subpass.depthResolveAttachmentReference) {
                SPDLOG_LOGGER_ERROR(Logger::logger(), "More than 1 Depth Resolve Attachment in this subpass!");
            }
            subpass.depthResolveAttachmentReference = { currentAttachmentDescriptionIndex++, depthAttachment.layout };
        }

        subpass.depthResolveMode = depthAttachment.depthResolveMode;
        subpass.stencilResolveMode = depthAttachment.stencilResolveMode;
    }

    return subpass;
}

// Called to create implicit RenderPass for GraphicsPipeline
SubpassDescription VulkanResourceManager::fillAttachmentDescriptionAndCreateSubpassDescription(std::vector<AttachmentDescription> &attachmentDescriptions,
                                                                                               const std::vector<RenderTargetOptions> &colorAttachments,
                                                                                               const DepthStencilOptions &depthStencil,
                                                                                               SampleCountFlagBits samples)
{
    const bool useMultiSampling = samples > SampleCountFlagBits::Samples1Bit;

    SubpassDescription subpass;
    uint32_t currentAttachmentDescriptionIndex = 0;

    for (const RenderTargetOptions &renderTarget : colorAttachments) {
        AttachmentDescription &attachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
        attachmentDescription.format = renderTarget.format;
        attachmentDescription.samples = samples;
        attachmentDescription.loadOperation = AttachmentLoadOperation::Clear;
        attachmentDescription.storeOperation = AttachmentStoreOperation::Store;
        attachmentDescription.stencilLoadOperation = AttachmentLoadOperation::DontCare;
        attachmentDescription.stencilStoreOperation = AttachmentStoreOperation::DontCare;
        attachmentDescription.initialLayout = TextureLayout::Undefined;
        attachmentDescription.finalLayout = useMultiSampling ? TextureLayout::ColorAttachmentOptimal : TextureLayout::PresentSrc;
        subpass.colorAttachmentReference.push_back({ currentAttachmentDescriptionIndex++, TextureLayout::ColorAttachmentOptimal });

        if (useMultiSampling) {
            AttachmentDescription &resolveAttachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
            resolveAttachmentDescription.format = renderTarget.format;
            resolveAttachmentDescription.samples = SampleCountFlagBits::Samples1Bit;
            resolveAttachmentDescription.loadOperation = AttachmentLoadOperation::Clear;
            resolveAttachmentDescription.storeOperation = AttachmentStoreOperation::Store;
            resolveAttachmentDescription.stencilLoadOperation = AttachmentLoadOperation::DontCare;
            resolveAttachmentDescription.stencilStoreOperation = AttachmentStoreOperation::DontCare;
            resolveAttachmentDescription.initialLayout = TextureLayout::Undefined;
            resolveAttachmentDescription.finalLayout = TextureLayout::PresentSrc;
            subpass.resolveAttachmentReference.push_back({ currentAttachmentDescriptionIndex++, TextureLayout::ColorAttachmentOptimal });
        }
    }

    const bool hasDepthAttachment = depthStencil.format != Format::UNDEFINED;
    const bool hasDepthResolveAttachment = hasDepthAttachment && useMultiSampling && depthStencil.resolveDepthStencil;

    if (hasDepthAttachment) {
        AttachmentDescription &attachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
        attachmentDescription.format = depthStencil.format;
        attachmentDescription.samples = samples;
        attachmentDescription.loadOperation = AttachmentLoadOperation::Clear;
        attachmentDescription.storeOperation = AttachmentStoreOperation::Store;
        attachmentDescription.stencilLoadOperation = AttachmentLoadOperation::DontCare;
        attachmentDescription.stencilStoreOperation = AttachmentStoreOperation::DontCare;
        attachmentDescription.initialLayout = TextureLayout::Undefined;
        attachmentDescription.finalLayout = TextureLayout::DepthStencilAttachmentOptimal;
        subpass.depthAttachmentReference = { currentAttachmentDescriptionIndex++, TextureLayout::DepthStencilAttachmentOptimal };

        if (hasDepthResolveAttachment) {
            AttachmentDescription &resolveAttachmentDescription = attachmentDescriptions.emplace_back(AttachmentDescription{});
            resolveAttachmentDescription.format = depthStencil.format;
            resolveAttachmentDescription.samples = SampleCountFlagBits::Samples1Bit;
            resolveAttachmentDescription.loadOperation = AttachmentLoadOperation::Clear;
            resolveAttachmentDescription.storeOperation = AttachmentStoreOperation::Store;
            resolveAttachmentDescription.stencilLoadOperation = AttachmentLoadOperation::DontCare;
            resolveAttachmentDescription.stencilStoreOperation = AttachmentStoreOperation::DontCare;
            resolveAttachmentDescription.initialLayout = TextureLayout::Undefined;
            resolveAttachmentDescription.finalLayout = TextureLayout::PresentSrc;
            subpass.depthResolveAttachmentReference = { currentAttachmentDescriptionIndex++, TextureLayout::DepthStencilAttachmentOptimal };
        }
    }

    return subpass;
}

Handle<TimestampQueryRecorder_t> VulkanResourceManager::createTimestampQueryRecorder(const Handle<Device_t> &deviceHandle,
                                                                                     const Handle<CommandRecorder_t> &commandRecorderHandle,
                                                                                     const TimestampQueryRecorderOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VulkanCommandRecorder *vulkanCommandRecorder = m_commandRecorders.get(commandRecorderHandle);
    if (!vulkanCommandRecorder) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Could not find a valid command recorder");
        return {};
    }
    VkCommandBuffer vkCommandBuffer = vulkanCommandRecorder->commandBuffer;

    if (vulkanDevice->timestampQueryPool == VK_NULL_HANDLE) {
        VkQueryPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        poolCreateInfo.queryCount = 1024;
        poolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        vkCreateQueryPool(vulkanDevice->device, &poolCreateInfo, nullptr, &vulkanDevice->timestampQueryPool);
    }

    // Find suitable starting index to accommodate requested number of queries
    uint32_t startQueryIndex = 0;
    bool addBucketToEnd = true;

    // Find where to insert new bucket (Buckets are always sorted by starting index)
    for (size_t i = 0, m = m_timestampQueryBuckets.size(); i < m; ++i) {
        const TimestampQueryBucket &b = m_timestampQueryBuckets[i];
        if (startQueryIndex + options.queryCount <= b.start) {
            m_timestampQueryBuckets.insert(m_timestampQueryBuckets.begin() + i, { startQueryIndex, options.queryCount });
            addBucketToEnd = false;
            break;
        }
        startQueryIndex = std::max(startQueryIndex, b.start + b.count);
    };
    if (addBucketToEnd)
        m_timestampQueryBuckets.push_back({ startQueryIndex, options.queryCount });

    const auto vulkanTimestampQueryRecorderHandle = m_timestampQueryRecorders.emplace(
            VulkanTimestampQueryRecorder(vkCommandBuffer, this, deviceHandle, startQueryIndex, options.queryCount));

    return vulkanTimestampQueryRecorderHandle;
}

void VulkanResourceManager::deleteTimestampQueryRecorder(const Handle<TimestampQueryRecorder_t> &handle)
{
    VulkanTimestampQueryRecorder *vulkanTimestampQueryRecorder = m_timestampQueryRecorders.get(handle);

    auto it = std::find_if(m_timestampQueryBuckets.begin(), m_timestampQueryBuckets.end(),
                           [vulkanTimestampQueryRecorder](const TimestampQueryBucket &b) {
                               return b.start == vulkanTimestampQueryRecorder->startQuery && b.count == vulkanTimestampQueryRecorder->maxQueryCount;
                           });
    assert(it != m_timestampQueryBuckets.end());
    m_timestampQueryBuckets.erase(it);

    // Keep buckets sorted by starting index
    std::sort(m_timestampQueryBuckets.begin(), m_timestampQueryBuckets.end(), [](const TimestampQueryBucket &a, const TimestampQueryBucket &b) {
        return a.start < b.start;
    });

    m_timestampQueryRecorders.remove(handle);
}

VulkanTimestampQueryRecorder *VulkanResourceManager::getTimestampQueryRecorder(const Handle<TimestampQueryRecorder_t> &handle) const
{
    return m_timestampQueryRecorders.get(handle);
}

namespace {
const std::vector<SubpassDependenciesDescriptions> defaultImplicitSubpassDependencies = {

    // Color & Depth Buffer Dependency to prevent clearing before previous renderpass has been completed
    // and allow to to read back from the attachments after previous pass has fully written into them.

    // This effectively combines 2 dependencies in 1:
    // 1) Layout transition → clear (write ordering) ensures:
    //    - layout transition (write)
    //    - clear (write)
    //   are correctly ordered to prevent a Write on Write Hazard
    // 2) Make writes (clear, fragment outputs) -> visible to reads in fragment shader
    //   (for input attachments / depth / color reads)
    {
            .srcSubpass = ExternalSubpass,
            .dstSubpass = 0,
            .srcStageMask = (PipelineStageFlagBit::ColorAttachmentOutputBit |
                             PipelineStageFlagBit::EarlyFragmentTestBit |
                             PipelineStageFlagBit::LateFragmentTestBit),
            .dstStageMask = (PipelineStageFlagBit::ColorAttachmentOutputBit |
                             PipelineStageFlagBit::EarlyFragmentTestBit |
                             PipelineStageFlagBit::LateFragmentTestBit |
                             PipelineStageFlagBit::FragmentShaderBit),
            .srcAccessMask = AccessFlagBit::None,
            .dstAccessMask = (AccessFlagBit::ColorAttachmentWriteBit |
                              AccessFlagBit::DepthStencilAttachmentWriteBit |
                              AccessFlagBit::InputAttachmentReadBit |
                              AccessFlagBit::ColorAttachmentReadBit |
                              AccessFlagBit::DepthStencilAttachmentReadBit),
    },
};

} // namespace

// Created from ColorAttachments & DepthStencilAttachment (RenderPassRecorder)
Handle<RenderPass_t> VulkanResourceManager::createImplicitRenderPass(const Handle<Device_t> &deviceHandle,
                                                                     const std::vector<ColorAttachment> &colorAttachments,
                                                                     const DepthStencilAttachment &depthStencilAttachment,
                                                                     SampleCountFlagBits samples, uint32_t viewCount)
{
    // default case, one subpass renderpass, assume one depth attachment for now
    const uint32_t multiViewMaskMask = uint32_t(1 << viewCount) - 1;

    std::vector<AttachmentDescription> attachmentDescriptions;
    SubpassDescription subpassDescription = fillAttachmentDescriptionAndCreateSubpassDescription(attachmentDescriptions, colorAttachments, depthStencilAttachment, samples);

    std::vector<uint32_t> viewMasks;
    if (viewCount > 1) {
        subpassDescription.viewMask = multiViewMaskMask;
        viewMasks = std::vector{ multiViewMaskMask };
    }

    return createRenderPass(deviceHandle, RenderPassOptions{
                                                  .attachments = attachmentDescriptions,
                                                  .subpassDescriptions = std::vector{ subpassDescription },
                                                  .subpassDependencies = defaultImplicitSubpassDependencies,
                                                  .correlatedViewMasks = viewMasks,
                                          });
}

// Created from RenderTarget descriptions (GraphicsPipeline)
Handle<RenderPass_t> VulkanResourceManager::createImplicitRenderPass(const Handle<Device_t> &deviceHandle,
                                                                     const std::vector<RenderTargetOptions> &colorAttachments,
                                                                     const DepthStencilOptions &depthAttachment,
                                                                     SampleCountFlagBits samples,
                                                                     uint32_t viewCount)
{

    // default case, one subpass renderpass, assume one depth attachment for now
    const uint32_t multiViewMaskMask = uint32_t(1 << viewCount) - 1;

    std::vector<AttachmentDescription> attachmentDescriptions;
    SubpassDescription subpassDescription = fillAttachmentDescriptionAndCreateSubpassDescription(attachmentDescriptions, colorAttachments, depthAttachment, samples);

    std::vector<uint32_t> viewMasks;
    if (viewCount > 1) {
        subpassDescription.viewMask = multiViewMaskMask;
        viewMasks = std::vector{ multiViewMaskMask };
    }

    return createRenderPass(deviceHandle, RenderPassOptions{
                                                  .attachments = attachmentDescriptions,
                                                  .subpassDescriptions = std::vector{ subpassDescription },
                                                  .subpassDependencies = defaultImplicitSubpassDependencies,
                                                  .correlatedViewMasks = viewMasks,
                                          });
}

Handle<Framebuffer_t> VulkanResourceManager::createFramebuffer(const Handle<Device_t> &deviceHandle,
                                                               const VulkanFramebufferKey &frameBufferKey)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkRenderPass vkRenderPass = m_renderPasses.get(frameBufferKey.renderPass)->renderPass;

    const uint32_t attachmentCount = frameBufferKey.attachmentsKey.handles.size();
    std::vector<VkImageView> vkAttachments;
    vkAttachments.reserve(attachmentCount);
    for (uint32_t i = 0; i < attachmentCount; ++i)
        vkAttachments.push_back(m_textureViews.get(frameBufferKey.attachmentsKey.handles.at(i))->imageView);

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass;
    framebufferInfo.attachmentCount = attachmentCount;
    framebufferInfo.pAttachments = vkAttachments.data();
    framebufferInfo.width = frameBufferKey.width;
    framebufferInfo.height = frameBufferKey.height;
    framebufferInfo.layers = frameBufferKey.layers;

    VkFramebuffer vkFramebuffer{ VK_NULL_HANDLE };
    if (auto result = vkCreateFramebuffer(vulkanDevice->device, &framebufferInfo, nullptr, &vkFramebuffer); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating framebuffer: {}", result);
        return {};
    }

    const auto vulkanFramebufferHandle = m_framebuffers.emplace(VulkanFramebuffer(vkFramebuffer, deviceHandle));
    return vulkanFramebufferHandle;
}

void VulkanResourceManager::deleteFramebuffer(const Handle<Framebuffer_t> &handle)
{
    VulkanFramebuffer *framebuffer = m_framebuffers.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(framebuffer->deviceHandle);

    vkDestroyFramebuffer(vulkanDevice->device, framebuffer->framebuffer, nullptr);

    m_framebuffers.remove(handle);
}

Handle<BindGroup_t> VulkanResourceManager::createBindGroup(const Handle<Device_t> &deviceHandle, const BindGroupOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    auto allocateDescriptorSet = [](VkDevice device, VkDescriptorPool descriptorPool,
                                    VulkanBindGroupLayout *bindGroupLayout, VkDescriptorSet &descriptorSet,
                                    uint32_t maxVariableDescriptorCounts) {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &bindGroupLayout->descriptorSetLayout;

        // For variable length descriptor arrays, this specify the maximum count we expect them to be.
        // Note that this value will apply to all bindings defined as variable arrays in the BindGroupLayout
        // used to allocate this BindGroup
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableLengthInfo{};
        if (maxVariableDescriptorCounts > 0) {
            variableLengthInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            variableLengthInfo.descriptorSetCount = 1;
            variableLengthInfo.pDescriptorCounts = &maxVariableDescriptorCounts;
            allocInfo.pNext = &variableLengthInfo;
        }

        return vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
    };

    // Determine which bind group pool to use
    Handle<BindGroupPool_t> poolHandle = options.bindGroupPool;
    const bool useInternalPool = !poolHandle.isValid();

    constexpr BindGroupPoolOptions defaultInternalPoolOptions{
        .label = "Default BindGroupPool",
        .uniformBufferCount = 512,
        .dynamicUniformBufferCount = 16,
        .storageBufferCount = 512,
        .textureSamplerCount = 128,
        .textureCount = 128,
        .samplerCount = 8,
        .imageCount = 8,
        .inputAttachmentCount = 8,
        .accelerationStructureCount = 8,
        .maxBindGroupCount = 1024,
        .flags = BindGroupPoolFlagBits::CreateFreeBindGroups
    };

    if (useInternalPool) {
        // Use or create a default pool from the device's pool vector
        if (vulkanDevice->descriptorSetPools.empty()) {
            // Create a default bind group pool with reasonable defaults
            vulkanDevice->descriptorSetPools.emplace_back(createBindGroupPool(deviceHandle, defaultInternalPoolOptions));
        }
        poolHandle = vulkanDevice->descriptorSetPools.back();
    }

    VulkanBindGroupPool *vulkanBindGroupPool = getBindGroupPool(poolHandle);
    VulkanBindGroupLayout *bindGroupLayout = getBindGroupLayout(options.layout);
    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

    // Allocate DescriptorSet from the bind group pool
    VkResult result = allocateDescriptorSet(vulkanDevice->device, vulkanBindGroupPool->descriptorPool,
                                            bindGroupLayout, descriptorSet, options.maxVariableArrayLength);

    // If we have run out of pool memory and rely on internalPools, create a new internal pool and retry
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        if (useInternalPool) {
            SPDLOG_LOGGER_INFO(Logger::logger(), "Internal BindGroup pool out of memory, creating additional pool");
            vulkanDevice->descriptorSetPools.emplace_back(createBindGroupPool(deviceHandle, defaultInternalPoolOptions));
            poolHandle = vulkanDevice->descriptorSetPools.back();
            vulkanBindGroupPool = getBindGroupPool(poolHandle);
            result = allocateDescriptorSet(vulkanDevice->device, vulkanBindGroupPool->descriptorPool,
                                           bindGroupLayout, descriptorSet, options.maxVariableArrayLength);
        } else {
            SPDLOG_LOGGER_ERROR(Logger::logger(), "BindGroupPool out of memory");
        }
    }

    if (result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when allocating descriptor set: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64_t>(descriptorSet), options.label);

    // Warn users that internalPools can't be reset/explicit destroyed since they have no user visible handles
    if (!options.implicitFree && useInternalPool) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "BindGroups with explicit free option must not use internal pools since these can't be reset. Please provide a valid BindGroupPool handle or set implicitFree to true.");
    }

    if (options.implicitFree && !vulkanBindGroupPool->flags.testFlag(BindGroupPoolFlagBits::CreateFreeBindGroups)) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "BindGroupPool does not support individual BindGroup free. Please change BindGroupPool creation flags or set implicitFree to false.");
    }

    const auto vulkanBindGroupHandle = m_bindGroups.emplace(VulkanBindGroup(descriptorSet,
                                                                            poolHandle,
                                                                            this,
                                                                            deviceHandle,
                                                                            options.implicitFree));
    // Record new bindgroup handle against pool
    vulkanBindGroupPool->addBindGroup(vulkanBindGroupHandle);

    // Set up the initial bindings
    auto vulkanBindGroup = m_bindGroups.get(vulkanBindGroupHandle);
    for (const auto &resource : options.resources)
        vulkanBindGroup->update(resource);

    return vulkanBindGroupHandle;
}

void VulkanResourceManager::deleteBindGroup(const Handle<BindGroup_t> &handle)
{
    VulkanBindGroup *vulkanBindGroup = m_bindGroups.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanBindGroup->deviceHandle);

    VulkanBindGroupPool *vulkanBindGroupPool = getBindGroupPool(vulkanBindGroup->bindGroupPoolHandle);

    // Destroy underlying Vulkan resource if still valid and bind group doesn't require explicit free
    if (vulkanBindGroup->descriptorSet != VK_NULL_HANDLE && vulkanBindGroup->implicitFree) {
        vkFreeDescriptorSets(vulkanDevice->device, vulkanBindGroupPool->descriptorPool, 1, &vulkanBindGroup->descriptorSet);

        // Remove the bind group handle from the bindGroupPool if using implicit free
        // If using explicit free, removing the bindGroup which hasn't been freed
        // would make BindGroupPool::allocatedBindGroupCount() return an incorrect value.
        vulkanBindGroupPool->removeBindGroup(handle);
    }

    m_bindGroups.remove(handle);
}

VulkanBindGroup *VulkanResourceManager::getBindGroup(const Handle<BindGroup_t> &handle) const
{
    return m_bindGroups.get(handle);
}

Handle<BindGroupLayout_t> VulkanResourceManager::createBindGroupLayout(const Handle<Device_t> &deviceHandle, const BindGroupLayoutOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    assert(options.bindings.size() <= std::numeric_limits<uint32_t>::max());
    const uint32_t bindingLayoutCount = static_cast<uint32_t>(options.bindings.size());
    std::vector<VkDescriptorSetLayoutBinding> vkBindingLayouts;
    std::vector<VkDescriptorBindingFlags> vkBindingFlags;
    vkBindingLayouts.reserve(bindingLayoutCount);
    vkBindingFlags.reserve(bindingLayoutCount);

    std::vector<VkSampler> immutableSamplers;

    for (uint32_t j = 0; j < bindingLayoutCount; ++j) {
        const auto &bindingLayout = options.bindings.at(j);

        VkDescriptorSetLayoutBinding vkBindingLayout = {};
        vkBindingLayout.binding = bindingLayout.binding;
        vkBindingLayout.descriptorCount = bindingLayout.count;
        vkBindingLayout.descriptorType = resourceBindingTypeToVkDescriptorType(bindingLayout.resourceType);
        vkBindingLayout.stageFlags = bindingLayout.shaderStages.toInt();
        vkBindingLayout.pImmutableSamplers = nullptr;

        if (!bindingLayout.immutableSamplers.empty()) {
            assert(bindingLayout.resourceType == ResourceBindingType::Sampler || bindingLayout.resourceType == ResourceBindingType::CombinedImageSampler);
            immutableSamplers.reserve(bindingLayout.immutableSamplers.size());
            const size_t lastImmutableSamplersOffset = immutableSamplers.size();
            for (Handle<Sampler_t> samplerH : bindingLayout.immutableSamplers) {
                VulkanSampler *s = m_samplers.get(samplerH);
                immutableSamplers.push_back(s->sampler);
            }
            vkBindingLayout.pImmutableSamplers = immutableSamplers.data() + lastImmutableSamplersOffset;
        };

        vkBindingLayouts.emplace_back(std::move(vkBindingLayout));

        VkDescriptorBindingFlags vkBindingFlag = resourceBindingFlagsToVkDescriptorBindingFlags(bindingLayout.flags);
        vkBindingFlags.emplace_back(std::move(vkBindingFlag));
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlags{};
    setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    setLayoutBindingFlags.bindingCount = static_cast<uint32_t>(vkBindingFlags.size());
    setLayoutBindingFlags.pBindingFlags = vkBindingFlags.data();

    // Associate the bindings into a descriptor set layout
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<uint32_t>(vkBindingLayouts.size());
    createInfo.pBindings = vkBindingLayouts.data();
    createInfo.pNext = &setLayoutBindingFlags;
    createInfo.flags = bindGroupLayoutFlagsToVkDescriptorSetLayoutCreateFlags(options.flags);

    VkDescriptorSetLayout vkDescriptorSetLayout{ VK_NULL_HANDLE };
    if (auto result = vkCreateDescriptorSetLayout(vulkanDevice->device, &createInfo, nullptr, &vkDescriptorSetLayout); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating pipeline layout: {}", result);
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, reinterpret_cast<uint64_t>(vkDescriptorSetLayout), options.label);

    const auto vulkanBindGroupLayoutHandle = m_bindGroupLayouts.emplace(VulkanBindGroupLayout(vkDescriptorSetLayout, deviceHandle, options.bindings));
    return vulkanBindGroupLayoutHandle;
}

void VulkanResourceManager::deleteBindGroupLayout(const Handle<BindGroupLayout_t> &handle)
{
    VulkanBindGroupLayout *vulkanBindGroupLayout = m_bindGroupLayouts.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(vulkanBindGroupLayout->deviceHandle);

    vkDestroyDescriptorSetLayout(vulkanDevice->device, vulkanBindGroupLayout->descriptorSetLayout, nullptr);

    m_bindGroupLayouts.remove(handle);
}

VulkanBindGroupLayout *VulkanResourceManager::getBindGroupLayout(const Handle<BindGroupLayout_t> &handle) const
{
    return m_bindGroupLayouts.get(handle);
}

Handle<Sampler_t> VulkanResourceManager::createSampler(const Handle<Device_t> &deviceHandle, const SamplerOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = filterModeToVkFilterMode(options.magFilter);
    samplerInfo.minFilter = filterModeToVkFilterMode(options.minFilter);

    samplerInfo.addressModeU = addressModeToVkSamplerAddressMode(options.u);
    samplerInfo.addressModeV = addressModeToVkSamplerAddressMode(options.v);
    samplerInfo.addressModeW = addressModeToVkSamplerAddressMode(options.w);

    samplerInfo.anisotropyEnable = options.anisotropyEnabled;
    samplerInfo.maxAnisotropy = options.maxAnisotropy;

    samplerInfo.compareEnable = options.compareEnabled;
    samplerInfo.compareOp = compareOperationToVkCompareOp(options.compare);

    samplerInfo.mipmapMode = mipMapFilterModeToVkSamplerMipmapMode(options.mipmapFilter);
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = options.lodMinClamp;
    samplerInfo.maxLod =
            options.lodMaxClamp == MipmapLodClamping::NoClamping
            ? VK_LOD_CLAMP_NONE
            : options.lodMaxClamp;

    samplerInfo.unnormalizedCoordinates = !options.normalizedCoordinates;

#if defined(VK_KHR_sampler_ycbcr_conversion)
    VkSamplerYcbcrConversionKHR yCbCrConversion{ VK_NULL_HANDLE };
    VkSamplerYcbcrConversionInfoKHR yCbCrInfo{ .sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO_KHR };

    if (options.yCbCrConversion.isValid()) {
        VulkanYCbCrConversion *vulkanConversion = m_yCbCrConversions.get(options.yCbCrConversion);
        assert(vulkanConversion);
        yCbCrConversion = vulkanConversion->yCbCrConversion;
        // Set Conversion Object on Sampler
        yCbCrInfo.conversion = yCbCrConversion;
        samplerInfo.pNext = &yCbCrInfo;
    }
#else
    assert(!options.yCbCrConversion.isValid());
#endif

    VkSampler sampler{ VK_NULL_HANDLE };
    if (auto result = vkCreateSampler(vulkanDevice->device, &samplerInfo, nullptr, &sampler); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating sampler: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(sampler), options.label);

    auto samplerHandle = m_samplers.emplace(VulkanSampler(sampler, deviceHandle));
    return samplerHandle;
}

void VulkanResourceManager::deleteSampler(const Handle<Sampler_t> &handle)
{
    VulkanSampler *sampler = m_samplers.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(sampler->deviceHandle);

    vkDestroySampler(vulkanDevice->device, sampler->sampler, nullptr);
    m_samplers.remove(handle);
}

VulkanSampler *VulkanResourceManager::getSampler(const Handle<Sampler_t> &handle) const
{
    return m_samplers.get(handle);
}

Handle<Fence_t> VulkanResourceManager::createFence(const Handle<Device_t> &deviceHandle, const FenceOptions &options)
{
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (options.createSignalled)
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkExportFenceCreateInfo exportFenceCreateInfo = {};
    if (options.externalFenceHandleType != ExternalFenceHandleTypeFlagBits::None) {
        exportFenceCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
        exportFenceCreateInfo.pNext = nullptr;
        exportFenceCreateInfo.handleTypes = externalFenceHandleTypeToVkExternalFenceHandleType(options.externalFenceHandleType);
        fenceInfo.pNext = &exportFenceCreateInfo;
    }

    VkFence vkFence{ VK_NULL_HANDLE };
    if (auto result = vkCreateFence(vulkanDevice->device, &fenceInfo, nullptr, &vkFence); result != VK_SUCCESS) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating fence: {}", result);
        return {};
    }

    HandleOrFD externalFenceHandle{};
    if (options.externalFenceHandleType == ExternalFenceHandleTypeFlagBits::OpaqueFD) {
#if defined(VK_KHR_external_fence_fd)
        if (vulkanDevice->vkGetFenceFdKHR) {
            VkFenceGetFdInfoKHR vulkanFenceGetFdInfoKHR = {
                .sType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR,
                .pNext = nullptr,
                .fence = vkFence,
                .handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT
            };
            int fd{};
            vulkanDevice->vkGetFenceFdKHR(vulkanDevice->device, &vulkanFenceGetFdInfoKHR, &fd);
            externalFenceHandle = fd;
        }
#else
        assert(false);
#endif
    } else if (options.externalFenceHandleType == ExternalFenceHandleTypeFlagBits::OpaqueWin32) {
#if defined(VK_KHR_external_fence_win32)
        if (vulkanDevice->vkGetFenceWin32HandleKHR) {
            VkFenceGetWin32HandleInfoKHR vulkanFenceGetHandleInfoKHR = {
                .sType = VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR,
                .pNext = nullptr,
                .fence = vkFence,
                .handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT
            };
            HANDLE winHandle{};
            vulkanDevice->vkGetFenceWin32HandleKHR(vulkanDevice->device, &vulkanFenceGetHandleInfoKHR, &winHandle);
            externalFenceHandle = winHandle;
        }
#else
        assert(false);
#endif
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_FENCE, reinterpret_cast<uint64_t>(vkFence), options.label);

    auto fenceHandle = m_fences.emplace(VulkanFence(vkFence, this, deviceHandle, externalFenceHandle));
    return fenceHandle;
}

void VulkanResourceManager::deleteFence(const Handle<Fence_t> &handle)
{
    VulkanFence *fence = m_fences.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(fence->deviceHandle);

    vkDestroyFence(vulkanDevice->device, fence->fence, nullptr);

    m_fences.remove(handle);
}

VulkanFence *VulkanResourceManager::getFence(const Handle<Fence_t> &handle) const
{
    return m_fences.get(handle);
}

void VulkanResourceManager::setObjectName(VulkanDevice *device, const VkObjectType type, const uint64_t handle, const std::string_view name)
{
#if defined(VK_EXT_debug_utils)
    if (device == nullptr || device->vkSetDebugUtilsObjectNameEXT == nullptr || name.empty()) {
        return;
    }
    const VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name.data()
    };
    device->vkSetDebugUtilsObjectNameEXT(device->device, &nameInfo);
#endif
}

Handle<AccelerationStructure_t> VulkanResourceManager::createAccelerationStructure(const Handle<Device_t> &deviceHandle, const AccelerationStructureOptions &options)
{
#if defined(VK_KHR_acceleration_structure)
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    std::vector<VkAccelerationStructureGeometryKHR> geometries;
    std::vector<uint32_t> maxPrimitiveCountPerGeometry;

    geometries.reserve(options.geometryTypesAndCount.size());
    maxPrimitiveCountPerGeometry.reserve(options.geometryTypesAndCount.size());

    for (const AccelerationStructureOptions::GeometryTypeAndCount &geometryTypeAndCount : options.geometryTypesAndCount) {
        VkAccelerationStructureGeometryKHR geometryKhr = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };

        std::visit([&geometryKhr](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, AccelerationStructureGeometryTrianglesData>) {
                VkAccelerationStructureGeometryTrianglesDataKHR trianglesDataKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
                trianglesDataKhr.vertexFormat = formatToVkFormat(arg.vertexFormat);
                trianglesDataKhr.vertexStride = arg.vertexStride;
                trianglesDataKhr.maxVertex = arg.maxVertex;
                trianglesDataKhr.indexType = indexTypeToVkIndexType(arg.indexType);

                geometryKhr.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                geometryKhr.geometry.triangles = trianglesDataKhr;
            } else if constexpr (std::is_same_v<T, AccelerationStructureGeometryInstancesData>) {
                VkAccelerationStructureGeometryInstancesDataKHR instancesDataKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };

                geometryKhr.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
                geometryKhr.geometry.instances = instancesDataKhr;
            } else if constexpr (std::is_same_v<T, AccelerationStructureGeometryAabbsData>) {
                VkAccelerationStructureGeometryAabbsDataKHR aabbsDataKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR };
                aabbsDataKhr.stride = arg.stride;

                geometryKhr.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
                geometryKhr.geometry.aabbs = aabbsDataKhr;
            } else {
                static_assert(always_false<T>, "non-exhaustive visitor!");
            }
        },
                   geometryTypeAndCount.geometry);

        geometries.push_back(geometryKhr);
        maxPrimitiveCountPerGeometry.push_back(geometryTypeAndCount.maxPrimitiveCount);
    }

    VkAccelerationStructureBuildGeometryInfoKHR geometryInfoKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
    geometryInfoKhr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    geometryInfoKhr.type = accelerationStructureTypeToVkAccelerationStructureType(options.type);
    geometryInfoKhr.pGeometries = geometries.data();
    geometryInfoKhr.geometryCount = geometries.size();
    geometryInfoKhr.flags = accelerationStructureFlagsToVkBuildAccelerationStructureFlags(options.flags);

    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfoKhr{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
    vulkanDevice->vkGetAccelerationStructureBuildSizesKHR(vulkanDevice->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &geometryInfoKhr, maxPrimitiveCountPerGeometry.data(), &buildSizesInfoKhr);

    // On AMD, updateScratchSize and buildScractchSize return 0
    // We therefore update these values to  accelerationStructureSize which is the size needed for a build or update
    if (buildSizesInfoKhr.updateScratchSize == 0)
        buildSizesInfoKhr.updateScratchSize = buildSizesInfoKhr.accelerationStructureSize;
    if (buildSizesInfoKhr.buildScratchSize == 0)
        buildSizesInfoKhr.buildScratchSize = buildSizesInfoKhr.accelerationStructureSize;

    Handle<Buffer_t> backingBufferH = VulkanAccelerationStructure::createAccelerationBuffer(deviceHandle, this, buildSizesInfoKhr.accelerationStructureSize);

    VulkanBuffer *backingBuffer = getBuffer(backingBufferH);

    VkAccelerationStructureCreateInfoKHR createInfoKhr = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
    createInfoKhr.size = buildSizesInfoKhr.accelerationStructureSize;
    createInfoKhr.buffer = backingBuffer->buffer;
    createInfoKhr.type = accelerationStructureTypeToVkAccelerationStructureType(options.type);

    VkAccelerationStructureKHR accelerationStructureKhr = VK_NULL_HANDLE;
    vulkanDevice->vkCreateAccelerationStructureKHR(vulkanDevice->device, &createInfoKhr, nullptr, &accelerationStructureKhr);

    auto accelerationStructureHandle = m_accelerationStructures.emplace(VulkanAccelerationStructure(deviceHandle, this, accelerationStructureKhr, backingBufferH, options.type, buildSizesInfoKhr, geometryInfoKhr.flags));

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, reinterpret_cast<uint64_t>(accelerationStructureKhr), options.label);

    return accelerationStructureHandle;
#else
    assert(false);
    return {};
#endif
}

void VulkanResourceManager::deleteAccelerationStructure(const Handle<AccelerationStructure_t> &handle)
{
#if defined(VK_KHR_acceleration_structure)
    VulkanAccelerationStructure *accelerationStructure = m_accelerationStructures.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(accelerationStructure->deviceHandle);

    vulkanDevice->vkDestroyAccelerationStructureKHR(vulkanDevice->device, accelerationStructure->accelerationStructure, nullptr);

    deleteBuffer(accelerationStructure->backingBuffer);

    m_accelerationStructures.remove(handle);
#endif
}

VulkanAccelerationStructure *VulkanResourceManager::getAccelerationStructure(const Handle<AccelerationStructure_t> &handle) const
{
    return m_accelerationStructures.get(handle);
}

Handle<YCbCrConversion_t> VulkanResourceManager::createYCbCrConversion(const Handle<Device_t> &deviceHandle, const YCbCrConversionOptions &options)
{
#if defined(VK_KHR_sampler_ycbcr_conversion)
    VulkanDevice *vulkanDevice = m_devices.get(deviceHandle);

    assert(vulkanDevice->requestedFeatures.samplerYCbCrConversion);

    VkSamplerYcbcrConversionKHR vkYCbCrConversion{ VK_NULL_HANDLE };

    VkSamplerYcbcrConversionCreateInfoKHR yCbCrConvInfo{};
    yCbCrConvInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO_KHR;
    yCbCrConvInfo.format = formatToVkFormat(options.format);
    yCbCrConvInfo.ycbcrModel = samplerYCbCrModelConvertionToVkSamplerYCbCrModelConversion(options.model);
    yCbCrConvInfo.ycbcrRange = samplerYCbCrRangeToVkSamplerYCbCrRange(options.range);
    yCbCrConvInfo.components = VkComponentMapping{
        .r = componentSwizzleToVkComponentSwizzle(options.components.r),
        .g = componentSwizzleToVkComponentSwizzle(options.components.g),
        .b = componentSwizzleToVkComponentSwizzle(options.components.b),
        .a = componentSwizzleToVkComponentSwizzle(options.components.a),
    };
    yCbCrConvInfo.xChromaOffset = chromaLocationToVkChromaLocation(options.xChromaOffset);
    yCbCrConvInfo.yChromaOffset = chromaLocationToVkChromaLocation(options.yChromaOffset);
    yCbCrConvInfo.chromaFilter = filterModeToVkFilterMode(options.chromaFilter);
    yCbCrConvInfo.forceExplicitReconstruction = options.forceExplicitReconstruction;

    // Create Conversion Object
    if (auto result = vulkanDevice->vkCreateSamplerYcbcrConversionKHR(vulkanDevice->device, &yCbCrConvInfo, nullptr, &vkYCbCrConversion)) {
        SPDLOG_LOGGER_ERROR(Logger::logger(), "Error when creating sampler Ycbcr conversion: {}", result);
        return {};
    }

    setObjectName(vulkanDevice, VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR, reinterpret_cast<uint64_t>(vkYCbCrConversion), options.label);

    auto conversionHandle = m_yCbCrConversions.emplace(VulkanYCbCrConversion(vkYCbCrConversion, deviceHandle));
    return conversionHandle;
#else
    return {};
#endif
}

void VulkanResourceManager::deleteYCbCrConversion(const Handle<YCbCrConversion_t> &handle)
{
#if defined(VK_KHR_sampler_ycbcr_conversion)
    VulkanYCbCrConversion *conversion = m_yCbCrConversions.get(handle);
    VulkanDevice *vulkanDevice = m_devices.get(conversion->deviceHandle);
    vulkanDevice->vkDestroySamplerYcbcrConversionKHR(vulkanDevice->device, conversion->yCbCrConversion, nullptr);
    m_yCbCrConversions.remove(handle);
#endif
}

VulkanYCbCrConversion *VulkanResourceManager::getYCbCrConversion(const Handle<YCbCrConversion_t> &handle) const
{
#if defined(VK_KHR_sampler_ycbcr_conversion)
    return m_yCbCrConversions.get(handle);
#endif
    return nullptr;
}

std::string VulkanResourceManager::getMemoryStats(const Handle<Device_t> &device) const
{
    VulkanDevice *vulkanDevice = m_devices.get(device);

    std::string stats;

    if (vulkanDevice->allocator) {
        char *statsAllocator = nullptr;
        vmaBuildStatsString(vulkanDevice->allocator, &statsAllocator, true);
        stats.append(statsAllocator);
        vmaFreeStatsString(vulkanDevice->allocator, statsAllocator);
    }

    for (const auto &[memoryHandleType, externalAllocator] : vulkanDevice->externalAllocators) {
        // Check if we have allocations in any of the heaps, otherwise don't build statistics
        const auto hasAllocations = [externalAllocator] {
            std::array<VmaBudget, VK_MAX_MEMORY_HEAPS> budgets;
            vmaGetHeapBudgets(externalAllocator, budgets.data());
            return std::any_of(budgets.begin(), budgets.end(), [](const VmaBudget &budget) {
                return budget.statistics.allocationCount > 0;
            });
        }();
        if (hasAllocations) {
            char *statsAllocator = nullptr;
            vmaBuildStatsString(externalAllocator, &statsAllocator, true);
            stats.append(statsAllocator);
            vmaFreeStatsString(externalAllocator, statsAllocator);
        }
    }

    return stats;
}

KDGpu::Format VulkanResourceManager::formatFromTextureView(const Handle<KDGpu::TextureView_t> &viewHandle) const
{
    VulkanTextureView *view = getTextureView(viewHandle);
    if (!view) {
        return KDGpu::Format::UNDEFINED;
    }
    VulkanTexture *texture = getTexture(view->textureHandle);
    if (!texture) {
        return KDGpu::Format::UNDEFINED;
    }

    return texture->format;
}

bool VulkanResourceManager::fillShaderStageInfos(const std::vector<ShaderStage> &stages,
                                                 ShaderStagesInfo &shaderStagesInfo)
{
    const uint32_t shaderCount = static_cast<uint32_t>(stages.size());
    shaderStagesInfo.shaderInfos.reserve(shaderCount);
    shaderStagesInfo.shaderSpecializationInfos.resize(shaderCount);
    shaderStagesInfo.shaderSpecializationMapEntries.resize(shaderCount);
    shaderStagesInfo.shaderSpecializationRawData.resize(shaderCount);

    for (uint32_t i = 0; i < shaderCount; ++i) {
        const auto &shaderStage = stages.at(i);

        VkPipelineShaderStageCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.stage = shaderStageFlagBitsToVkShaderStageFlagBits(shaderStage.stage);

        // Lookup the shader module
        const auto vulkanShaderModule = getShaderModule(shaderStage.shaderModule);
        if (!vulkanShaderModule)
            return false;
        shaderInfo.module = vulkanShaderModule->shaderModule;
        shaderInfo.pName = shaderStage.entryPoint.data();

        // Specialization Constants
        if (!shaderStage.specializationConstants.empty()) {
            std::vector<VkSpecializationMapEntry> &specializationConstantEntries = shaderStagesInfo.shaderSpecializationMapEntries[i];
            std::vector<uint8_t> &specializationRawData = shaderStagesInfo.shaderSpecializationRawData[i];
            uint32_t byteOffset = 0;

            const size_t specializationConstantsCount = shaderStage.specializationConstants.size();
            specializationConstantEntries.reserve(specializationConstantsCount);

            for (size_t sCI = 0; sCI < specializationConstantsCount; ++sCI) {
                const SpecializationConstant &specializationConstant = shaderStage.specializationConstants[sCI];
                const SpecializationConstantData &specializationConstantData = getByteOffsetSizeAndRawValueForSpecializationConstant(specializationConstant);

                specializationConstantEntries.emplace_back(VkSpecializationMapEntry{
                        .constantID = specializationConstant.constantId,
                        .offset = byteOffset,
                        .size = specializationConstantData.byteSize,
                });

                // Append Raw Byte Values
                const std::vector<uint8_t> &rawData = specializationConstantData.byteValues;
                specializationRawData.insert(specializationRawData.end(), rawData.begin(), rawData.end());

                // Increase offset
                byteOffset += specializationConstantData.byteSize;
            }

            VkSpecializationInfo &specializationInfo = shaderStagesInfo.shaderSpecializationInfos[i];
            specializationInfo.mapEntryCount = specializationConstantsCount;
            specializationInfo.pMapEntries = specializationConstantEntries.data();
            specializationInfo.dataSize = specializationRawData.size();
            specializationInfo.pData = specializationRawData.data();
            shaderInfo.pSpecializationInfo = &specializationInfo;
        }

        shaderStagesInfo.shaderInfos.emplace_back(shaderInfo);
    }
    return true;
}

std::vector<std::string> VulkanResourceManager::getAvailableLayers() const
{
    uint32_t layerCount{ 0 };
    if (auto result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr); result != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to enumerate instance layers: {}", result);
        return {};
    }

    std::vector<VkLayerProperties> vkLayers(layerCount);
    if (auto result = vkEnumerateInstanceLayerProperties(&layerCount, vkLayers.data()); result != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to query instance layers: {}", result);
        return {};
    }

    std::vector<std::string> layers;
    layers.reserve(layerCount);
    for (const auto &vkExtension : vkLayers) {
        layers.push_back(vkExtension.layerName);
    }

    return layers;
}

MemoryHandle VulkanResourceManager::retrieveExternalMemoryHandle(VulkanInstance *instance,
                                                                 VulkanDevice *vulkanDevice,
                                                                 const VmaAllocationInfo &allocationInfo,
                                                                 ExternalMemoryHandleTypeFlags handleType) const
{
    MemoryHandle memoryHandle{
        .allocationSize = allocationInfo.size,
        .allocationOffset = allocationInfo.offset,
    };
#if !defined(KDGPU_PLATFORM_WIN32)
    if (testIfContainsAnyFlags(handleType,
                               {
                                       ExternalMemoryHandleTypeFlagBits::OpaqueFD,
#if defined(VK_EXT_external_memory_dma_buf)
                                       ExternalMemoryHandleTypeFlagBits::DmaBuf,
#endif
                               })) {
#if defined(VK_KHR_external_memory_fd)
        if (instance->vkGetMemoryFdKHR) {
            VkMemoryGetFdInfoKHR vkMemoryGetFdInfoKHR = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
                .pNext = nullptr,
                .memory = allocationInfo.deviceMemory,
                .handleType = externalMemoryHandleTypeToVkExternalMemoryHandleType(handleType)
            };
            int fd{};
            instance->vkGetMemoryFdKHR(vulkanDevice->device, &vkMemoryGetFdInfoKHR, &fd);
            memoryHandle.handle = fd;
        }
#else
        assert(false);
#endif // VK_KHR_external_memory_fd
    }

#else // KDGPU_PLATFORM_WIN32

    if (testIfContainsAnyFlags(handleType,
                               {
                                       ExternalMemoryHandleTypeFlagBits::OpaqueWin32,
                                       ExternalMemoryHandleTypeFlagBits::OpaqueWin32Kmt,
                                       ExternalMemoryHandleTypeFlagBits::D3D11Texture,
                                       ExternalMemoryHandleTypeFlagBits::D3D11TextureKmt,
                                       ExternalMemoryHandleTypeFlagBits::D3D12Heap,
                                       ExternalMemoryHandleTypeFlagBits::D3D12Resource,
                               })) {
#if defined(VK_KHR_external_memory_win32)
        if (instance->vkGetMemoryWin32HandleKHR) {
            VkMemoryGetWin32HandleInfoKHR vkGetWin32HandleInfoKHR = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR,
                .pNext = nullptr,
                .memory = allocationInfo.deviceMemory,
                .handleType = externalMemoryHandleTypeToVkExternalMemoryHandleType(handleType)
            };
            HANDLE winHandle{};
            instance->vkGetMemoryWin32HandleKHR(vulkanDevice->device, &vkGetWin32HandleInfoKHR, &winHandle);
            memoryHandle.handle = winHandle;
        }
#else
        assert(false);
#endif // VK_KHR_external_memory_win32
    }

#endif // KDGPU_PLATFORM_WIN32

    else {
        assert(false); // Unsupported Handle Type
    }

    return memoryHandle;
}

} // namespace KDGpu
