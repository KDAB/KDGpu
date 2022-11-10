#include "device.h"

#include <toy_renderer/adapter.h>
#include <toy_renderer/graphics_api.h>
#include <toy_renderer/swapchain_options.h>
#include <toy_renderer/api/api_device.h>

namespace ToyRenderer {

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

Device::~Device()
{
}

Swapchain Device::createSwapchain(const SwapchainOptions &options)
{
    return Swapchain(m_api, m_device, m_api->resourceManager()->createSwapchain(m_device, options));
}

} // namespace ToyRenderer
