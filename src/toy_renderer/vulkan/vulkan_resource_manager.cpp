#include "vulkan_resource_manager.h"

#include <toy_renderer/instance.h>
#include <toy_renderer/vulkan/vulkan_config.h>

#include <assert.h>
#include <stdexcept>

namespace ToyRenderer {

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
    appInfo.pEngineName = "Serenity Prototype";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (!requestedInstanceLayers.empty()) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(requestedInstanceLayers.size());
        assert(requestedInstanceLayers.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledLayerNames = requestedInstanceLayers.data();
    }

    const auto requestedInstanceExtensions = getRequestedInstanceExtensions();
    if (!requestedInstanceExtensions.empty()) {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requestedInstanceExtensions.size());
        assert(requestedInstanceExtensions.size() <= std::numeric_limits<uint32_t>::max());
        createInfo.ppEnabledExtensionNames = requestedInstanceExtensions.data();
    }

    // Try to create the instance
    VkInstance instance = VK_NULL_HANDLE;
    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }

    VulkanInstance vulkanInstance(this, instance);
    auto h = m_instances.emplace(vulkanInstance);
    return h;
}

void VulkanResourceManager::deleteInstance(Handle<Instance_t> handle)
{
    // TODO: Implement me!
}

Handle<Adapter_t> VulkanResourceManager::insertAdapter(const VulkanAdapter &physicalDevice)
{
    return m_adapters.emplace(physicalDevice);
}

void VulkanResourceManager::removeAdapter(Handle<Adapter_t> handle)
{
    m_adapters.remove(handle);
}

/*
 * Create a VkDevice (logical device) from the provided adapter (physical device) and requested options.
 * If no options are specified we request a single queue from the first family (usually graphics capable).
 */
Handle<Device_t> VulkanResourceManager::createDevice(const Handle<Adapter_t> &adapterHandle, const DeviceOptions &options)
{
    std::vector<QueueRequest> queueRequests = options.queues;
    if (queueRequests.empty()) {
        QueueRequest queueRequest = {
            .familyIndex = 0,
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
        queueCreateInfo.queueFamilyIndex = queueRequest.familyIndex;
        queueCreateInfo.queueCount = queueRequest.count;
        queueCreateInfo.pQueuePriorities = queueRequest.priorities.data();
        
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // TODO: Obey requested adapter features (e.g. geometry shaders)
    // TODO: Obey requested device extensions and layers
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr; // TODO: Use VkPhysicalDeviceFeatures2
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;

    VkDevice vkDevice { VK_NULL_HANDLE };
    VulkanAdapter vulkanAdapter = *getAdapter(adapterHandle);
    if (vkCreateDevice(vulkanAdapter.physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a logical device!");
        
    const auto deviceHandle = m_devices.emplace(vkDevice);
        
    // TODO: Get the queues
    
    return deviceHandle;
}

void VulkanResourceManager::deleteDevice(Handle<Device_t> handle)
{
    // TODO: Implement me!
}

Handle<BindGroup> VulkanResourceManager::createBindGroup(BindGroupDescription desc)
{
    // TODO: This is where we will call vkAllocateDescriptorSets
    return {};
}

void VulkanResourceManager::deleteBindGroup(Handle<BindGroup> handle)
{
    // TODO: Implement me!
}

} // namespace ToyRenderer
