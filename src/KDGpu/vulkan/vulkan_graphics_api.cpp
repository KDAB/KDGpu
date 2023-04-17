#include "vulkan_graphics_api.h"

#include <KDGpu/vulkan/vulkan_enums.h>

namespace KDGpu {

VulkanGraphicsApi::VulkanGraphicsApi()
    : GraphicsApi()
    , m_vulkanResourceManager{ std::make_unique<VulkanResourceManager>() }
{
    m_resourceManager = m_vulkanResourceManager.get();
}

VulkanGraphicsApi::~VulkanGraphicsApi()
{
}

Instance VulkanGraphicsApi::createInstanceFromExistingVkInstance(VkInstance vkInstance)
{
    Instance instance;
    instance.m_api = this;
    instance.m_instance = m_vulkanResourceManager->createInstanceFromExistingVkInstance(vkInstance);
    return instance;
}

Surface VulkanGraphicsApi::createSurfaceFromExistingVkSurface(const Handle<Instance_t> &instanceH, VkSurfaceKHR vkSurface)
{
    VulkanInstance *instance = m_vulkanResourceManager->getInstance(instanceH);
    return Surface(this, instance->createSurface(vkSurface));
}

Adapter VulkanGraphicsApi::createAdapterFromExistingVkPhysicalDevice(const Handle<Instance_t> &instanceH, VkPhysicalDevice vkPhysicalDevice)
{
    return Adapter(this,
                   m_vulkanResourceManager->insertAdapter(
                           VulkanAdapter(vkPhysicalDevice, m_vulkanResourceManager.get(), instanceH)));
}

Queue VulkanGraphicsApi::createQueueFromExistingVkQueue(VkQueue vkQueue, const QueueFlags queueFlags)
{
    const Handle<Queue_t> queueHandle = m_vulkanResourceManager->insertQueue(VulkanQueue(vkQueue, m_vulkanResourceManager.get()));
    return Queue(this,
                 QueueDescription{
                         .queue = queueHandle,
                         .flags = queueFlags,
                         .queueTypeIndex = 0,
                         // We can't deduce the other Fields
                 });
}

Device VulkanGraphicsApi::createDeviceFromExisitingVkDevice(Adapter *adapter,
                                                            VkDevice vkDevice,
                                                            std::vector<Queue> &&queues)
{
    Device device;
    device.m_api = this;
    device.m_adapter = adapter;
    device.m_device = m_vulkanResourceManager->createDeviceFromExisitingVkDevice(adapter->handle(), vkDevice);
    device.m_queues = std::move(queues);

    // Note: we can't know what queues the VkDevice was create with, we assume createQueueFromExistingVkQueue
    // will be called by the user to create the queues he needs and passed in to this function

    // Copy the Queue Description into the VulkanDevice as that might be used
    // by the CommandRecorders to resolve which queue to use
    VulkanDevice *vulkanDevice = m_vulkanResourceManager->getDevice(device.m_device);
    assert(vulkanDevice);
    std::vector<QueueDescription> descriptions;
    descriptions.reserve(device.m_queues.size());
    for (const Queue &queue : device.m_queues)
        descriptions.push_back(QueueDescription{
                .queue = queue.handle(),
                .flags = queue.flags(),
                .timestampValidBits = queue.timestampValidBits(),
                .minImageTransferGranularity = queue.minImageTransferGranularity(),
                .queueTypeIndex = queue.queueTypeIndex(),
        });
    vulkanDevice->queueDescriptions = std::move(descriptions);

    return device;
}

} // namespace KDGpu
