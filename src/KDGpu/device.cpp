/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "device.h"

#include <KDGpu/adapter.h>
#include <KDGpu/device_options.h>
#include <KDGpu/api/graphics_api_impl.h>
#include <KDGpu/swapchain_options.h>

namespace KDGpu {

/**
    @class Device
    @brief Device is our main entry point to create Graphics Resources
    @ingroup public
    @headerfile device.h <KDGpu/device.h>

    @code{.cpp}
    using namespace KDGpu;

    Adapter *selectedAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    Device device = selectedAdapter->createDevice();
    @endcode

    @sa Adapter::createDevice
 */

/**
    @fn Device::handle()
    @brief Returns the handle used to retrieve the underlying API specific Device

    @sa ResourceManager
 */

/**
    @fn Device::isValid()
    @brief Convenience function to check whether the Device is actually referencing a valid API specific resource
 */

/**
    @fn Device::queues()
    @brief Returns the queues available on the device
 */

Device::Device()
    : m_api(nullptr)
{
}

Device::Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options)
    : m_api(api)
    , m_adapter(adapter)
{
    // Pass in a vector of queue requests which will be populated with the actual set of
    // queues requested by the device creation.
    std::vector<QueueRequest> queueRequests;
    m_device = m_api->resourceManager()->createDevice(adapter->handle(), options, queueRequests);
    auto apiDevice = m_api->resourceManager()->getDevice(m_device);

    // To fetch the queues from the device we pass in the actual set of queue requests so that
    // we can match up the queues to the queue family indices and other properties
    const auto queueDescriptions = apiDevice->getQueues(m_api->resourceManager(), queueRequests, adapter->queueTypes());
    const uint32_t queueCount = queueDescriptions.size();
    m_queues.reserve(queueCount);
    for (uint32_t i = 0; i < queueCount; ++i)
        m_queues.emplace_back(Queue(m_api, m_device, queueDescriptions[i]));
}

Device::Device(Device &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_queues = std::exchange(other.m_queues, {});
    m_adapter = std::exchange(other.m_adapter, {});
}

Device &Device::operator=(Device &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteDevice(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_queues = std::exchange(other.m_queues, {});
        m_adapter = std::exchange(other.m_adapter, {});
    }
    return *this;
}

Device::~Device()
{
    if (isValid())
        m_api->resourceManager()->deleteDevice(handle());
}

const Adapter *Device::adapter() const
{
    return m_adapter;
}

/**
 * @brief Forces a CPU side blocking wait until the underlying device has completed execution of all its pending commands.
 */
void Device::waitUntilIdle()
{
    auto apiDevice = m_api->resourceManager()->getDevice(m_device);
    apiDevice->waitUntilIdle();
}

Swapchain Device::createSwapchain(const SwapchainOptions &options)
{
    return Swapchain(m_api, m_device, options);
}

Texture Device::createTexture(const TextureOptions &options)
{
    return Texture(m_api, m_device, options);
}

Buffer Device::createBuffer(const BufferOptions &options, const void *initialData)
{
    return Buffer(m_api, m_device, options, initialData);
}

ShaderModule Device::createShaderModule(const std::vector<uint32_t> &code)
{
    return ShaderModule(m_api, m_device, code);
}

RenderPass Device::createRenderPass(const RenderPassOptions &options)
{
    return RenderPass(m_api, m_device, options);
}

PipelineLayout Device::createPipelineLayout(const PipelineLayoutOptions &options)
{
    return PipelineLayout(m_api, m_device, options);
}

GraphicsPipeline Device::createGraphicsPipeline(const GraphicsPipelineOptions &options)
{
    return GraphicsPipeline(m_api, m_device, options);
}

ComputePipeline Device::createComputePipeline(const ComputePipelineOptions &options)
{
    return ComputePipeline(m_api, m_device, options);
}

RayTracingPipeline Device::createRayTracingPipeline(const RayTracingPipelineOptions &options)
{
    return RayTracingPipeline(m_api, m_device, options);
}

CommandRecorder Device::createCommandRecorder(const CommandRecorderOptions &options)
{
    return CommandRecorder(m_api, m_device, options);
}

GpuSemaphore Device::createGpuSemaphore(const GpuSemaphoreOptions &options)
{
    return GpuSemaphore(m_api, m_device, options);
}

BindGroupLayout Device::createBindGroupLayout(const BindGroupLayoutOptions &options)
{
    return BindGroupLayout(m_api, m_device, options);
}

BindGroup Device::createBindGroup(const BindGroupOptions &options)
{
    return BindGroup(m_api, m_device, options);
}

Sampler Device::createSampler(const SamplerOptions &options)
{
    return Sampler(m_api, m_device, options);
}

Fence Device::createFence(const FenceOptions &options)
{
    return Fence(m_api, m_device, options);
}

AccelerationStructure Device::createAccelerationStructure(const KDGpu::AccelerationStructureOptions &options)
{
    return AccelerationStructure(m_api, m_device, options);
}

YCbCrConversion Device::createYCbCrConversion(const YCbCrConversionOptions &options)
{
    return YCbCrConversion(m_api, m_device, options);
}

GraphicsApi *Device::graphicsApi() const
{
    return m_api;
}

} // namespace KDGpu
