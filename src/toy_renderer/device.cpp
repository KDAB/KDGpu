#include "device.h"

#include <toy_renderer/adapter.h>
#include <toy_renderer/graphics_api.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/api/api_device.h>
#include <toy_renderer/bind_group_options.h>
#include <toy_renderer/bind_group_layout_options.h>

namespace ToyRenderer {

Device::Device()
    : m_api(nullptr)
{
}

Device::Device(Adapter *adapter, GraphicsApi *api, const DeviceOptions &options)
    : m_api(api)
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
        m_queues.emplace_back(Queue(m_api, queueDescriptions[i]));
}

Device::Device(Device &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_queues = std::move(other.m_queues);

    other.m_api = nullptr;
    other.m_device = {};
    other.m_queues = {};
}

Device &Device::operator=(Device &&other)
{
    if (this != &other) {
        m_api = other.m_api;
        m_device = other.m_device;
        m_queues = std::move(other.m_queues);

        other.m_api = nullptr;
        other.m_device = {};
        other.m_queues = {};
    }
    return *this;
}

Device::~Device()
{
    if (isValid())
        m_api->resourceManager()->deleteDevice(handle());
}

void Device::waitUntilIdle()
{
    auto apiDevice = m_api->resourceManager()->getDevice(m_device);
    apiDevice->waitUntilIdle();
}

Swapchain Device::createSwapchain(const SwapchainOptions &options)
{
    return Swapchain(m_api, m_device, m_api->resourceManager()->createSwapchain(m_device, options));
}

Texture Device::createTexture(const TextureOptions &options)
{
    return Texture(m_api, m_device, m_api->resourceManager()->createTexture(m_device, options));
}

Buffer Device::createBuffer(const BufferOptions &options, const void *initialData)
{
    return Buffer(m_api, m_device, m_api->resourceManager()->createBuffer(m_device, options, initialData));
}

ShaderModule Device::createShaderModule(const std::vector<uint32_t> &code)
{
    return ShaderModule(m_api, m_device, m_api->resourceManager()->createShaderModule(m_device, code));
}

PipelineLayout Device::createPipelineLayout(const PipelineLayoutOptions &options)
{
    return PipelineLayout(m_api, m_device, m_api->resourceManager()->createPipelineLayout(m_device, options));
}

GraphicsPipeline Device::createGraphicsPipeline(const GraphicsPipelineOptions &options)
{
    return GraphicsPipeline(m_api, m_device, m_api->resourceManager()->createGraphicsPipeline(m_device, options));
}

ComputePipeline Device::createComputePipeline(const ComputePipelineOptions &options)
{
    return ComputePipeline(m_api, m_device, m_api->resourceManager()->createComputePipeline(m_device, options));
}

CommandRecorder Device::createCommandRecorder(const CommandRecorderOptions &options)
{
    return CommandRecorder(m_api, m_device, m_api->resourceManager()->createCommandRecorder(m_device, options));
}

GpuSemaphore Device::createGpuSemaphore(const GpuSemaphoreOptions &options)
{
    return GpuSemaphore(m_api, m_device, m_api->resourceManager()->createGpuSemaphore(m_device, options));
}

BindGroupLayout Device::createBindGroupLayout(const BindGroupLayoutOptions &options)
{
    return BindGroupLayout(m_api, m_device, m_api->resourceManager()->createBindGroupLayout(m_device, options));
}

BindGroup Device::createBindGroup(const BindGroupOptions &options)
{
    return BindGroup(m_api, m_device, m_api->resourceManager()->createBindGroup(m_device, options));
}

Sampler Device::createSampler(const SamplerOptions &options)
{
    return Sampler(m_api, m_device, m_api->resourceManager()->createSampler(m_device, options));
}

} // namespace ToyRenderer
