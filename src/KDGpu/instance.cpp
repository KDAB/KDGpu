/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "instance.h"

#include <KDGpu/graphics_api.h>

#include <KDGpu/utils/logging.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <KDGpu/api/graphics_api_impl.h>
#include <KDGpu/api/resource_manager_impl.h>

namespace KDGpu {

/**
    @class Instance
    @brief Instance is used to initialize the Rendering API.
    @ingroup public
    @headerfile instance.h <KDGpu/instance.h>

    @code{.cpp}
    using namespace KDGpu;

    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

    Instance instance = api->createInstance(InstanceOptions{
            .applicationName = "MyApplication",
            .applicationVersion = 0,
    });
    @endcode

    @sa InstanceOptions
    @sa GraphicsApi::createInstance
 */

/**
    @fn Instance::handle()
    @brief Returns the handle used to retrieve the underlying Rendering API specific Instance

    @return Handle<Instance_t>
    @sa ResourceManager
 */

/**
    @fn Instance::isValid()
    @brief Convenience function to check whether the Instance is actually referencing a valid API specific resource
 */

Instance::Instance()
{
}

Instance::Instance(GraphicsApi *api, const InstanceOptions &options)
{
    // Create an instance using the underlying API
    m_api = api;
    m_instance = m_api->resourceManager()->createInstance(options);
}

Instance::~Instance()
{
    if (isValid())
        m_api->resourceManager()->deleteInstance(handle());
}

Instance::Instance(Instance &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_instance = std::exchange(other.m_instance, {});
    m_adapters = std::exchange(other.m_adapters, {});
}

Instance &Instance::operator=(Instance &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteInstance(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_instance = std::exchange(other.m_instance, {});
        m_adapters = std::exchange(other.m_adapters, {});
    }
    return *this;
}

/**
 * @brief Returns the extensions requested for the instance
 */
std::vector<Extension> Instance::extensions() const
{
    auto *apiInstance = m_api->resourceManager()->getInstance(m_instance);
    return apiInstance->extensions();
}

/**
 * @brief Convenience function used to create a Device instance that supports presentation against Surface @a surface
 */
AdapterAndDevice Instance::createDefaultDevice(const Surface &surface,
                                               AdapterDeviceType deviceType) const
{
    // Enumerate the adapters (physical devices) and select one to use. Here we look for
    // a discrete GPU. In a real app, we could fallback to an integrated one.
    Adapter *selectedAdapter = selectAdapter(deviceType);
    if (!selectedAdapter) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to find a suitable Adapter. Aborting...");
        return {};
    }

    auto queueTypes = selectedAdapter->queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
    SPDLOG_LOGGER_INFO(Logger::logger(), "Queue family 0 graphics and compute support: {}", hasGraphicsAndCompute);

    // We are now able to query the adapter for swapchain properties and presentation support with the window surface
    const auto swapchainProperties = selectedAdapter->swapchainProperties(surface);
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supported swapchain present modes:");
    for (const auto &mode : swapchainProperties.presentModes) {
        SPDLOG_LOGGER_INFO(Logger::logger(), "  - {}", presentModeToString(mode));
    }

    const bool supportsPresentation = selectedAdapter->supportsPresentation(surface, 0); // Query about the 1st queue type
    SPDLOG_LOGGER_INFO(Logger::logger(), "Queue family 0 supports presentation: {}", supportsPresentation);

    const auto adapterExtensions = selectedAdapter->extensions();
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supported adapter extensions:");
    for (const auto &extension : adapterExtensions) {
        SPDLOG_LOGGER_INFO(Logger::logger(), "  - {} Version {}", extension.name, extension.version);
    }

    if (!supportsPresentation || !hasGraphicsAndCompute) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Selected adapter queue family 0 does not meet requirements. Aborting.");
        return {};
    }

    const bool supportsMultiView = selectedAdapter->features().multiView;
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports multiview: {}", supportsMultiView);

    const bool supportsUBOIndexing = selectedAdapter->features().shaderUniformBufferArrayNonUniformIndexing && selectedAdapter->features().bindGroupBindingUniformBufferUpdateAfterBind;
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports Uniform Bind Group Dynamic Indexing: {}", supportsUBOIndexing);

    const bool supportsAccelerationStructures = selectedAdapter->features().accelerationStructures;
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports acceleration structures: {}", supportsAccelerationStructures);

    const bool supportsRayTracing = selectedAdapter->features().rayTracingPipeline;
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports raytracing: {}", supportsRayTracing);

    const bool supportsMeshShader = selectedAdapter->features().meshShader;
    const bool supportsTaskShader = selectedAdapter->features().taskShader;
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports meshShader: {}", supportsMeshShader);
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports taskShader: {}", supportsTaskShader);

    const bool supportsHostToImageCopy = selectedAdapter->features().hostImageCopy;
    SPDLOG_LOGGER_INFO(Logger::logger(), "Supports host to image copy: {}", supportsHostToImageCopy);

    // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    auto device = selectedAdapter->createDevice(DeviceOptions{
            .requestedFeatures = selectedAdapter->features() });

    return { selectedAdapter, std::move(device) };
}

/**
 * @brief Returns a vector of the Adapter instances available for the instance
 */
std::vector<Adapter *> Instance::adapters() const
{
    if (m_adapters.empty()) {
        auto *apiInstance = m_api->resourceManager()->getInstance(m_instance);
        // TODO: If we could look up a handle from a value, we would not need to pass m_instance into
        // queryAdapters(). It is needed so the adapter can store the instance handle for later use
        // when a device needs it to create a VMA allocator.
        auto adapterHandles = apiInstance->queryAdapters(m_instance);
        const auto adapterCount = static_cast<uint32_t>(adapterHandles.size());
        m_adapters.reserve(adapterCount);
        for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
            m_adapters.emplace_back(Adapter{ m_api, adapterHandles[adapterIndex] });

        m_adapterGroups = apiInstance->queryAdapterGroups();
    }

    std::vector<Adapter *> adapterPtrs;
    adapterPtrs.resize(m_adapters.size());
    for (size_t i = 0, m = m_adapters.size(); i < m; ++i)
        adapterPtrs[i] = m_adapters.data() + i;

    return adapterPtrs;
}

/**
 * @brief Returns a vector of the AdapterGroups available for the instance.
 * AdapterGroup allow to spread operations across multiple adapters;
 */
const std::vector<AdapterGroup> &Instance::adapterGroups() const
{
    if (m_adapters.empty())
        std::ignore = adapters();
    return m_adapterGroups;
}

/**
 * @brief Convenience function to easily select an Adapter instance
 */
Adapter *Instance::selectAdapter(AdapterDeviceType deviceType) const
{
    std::vector<AdapterDeviceType> lookupTypes;
    if (deviceType == KDGpu::AdapterDeviceType::Default) {
        lookupTypes.push_back(AdapterDeviceType::DiscreteGpu);
        lookupTypes.push_back(AdapterDeviceType::IntegratedGpu);
    } else {
        lookupTypes.push_back(deviceType);
    }
    std::vector<Adapter *> adaptersList = adapters();
    for (auto t : lookupTypes) {
        for (Adapter *adapter : adaptersList) {
            const AdapterProperties &properties = adapter->properties();
            if (properties.deviceType == t)
                return adapter;
        }
    }
    return nullptr;
}

/**
 * @brief Create a Surface instance based on the provided options
 *
 */
Surface Instance::createSurface(const SurfaceOptions &options)
{
    auto *apiInstance = m_api->resourceManager()->getInstance(m_instance);
    return Surface(m_api, apiInstance->createSurface(options));
}

} // namespace KDGpu
