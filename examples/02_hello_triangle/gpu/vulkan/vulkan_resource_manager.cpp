#include "vulkan_resource_manager.h"
#include "vulkan_config.h"

#include <assert.h>
#include <stdexcept>

namespace Gpu {

VulkanResourceManager::VulkanResourceManager()
{
    ms_instance = this;
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

    auto h = m_instances.emplace(instance);
    return h;
}

void VulkanResourceManager::deleteInstance(Handle<Instance_t> handle)
{
}

uint32_t VulkanResourceManager::adapterCount(const Handle<Instance_t> &instance) const
{
    VkInstance vkInstance = *m_instances.get(instance);
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    return deviceCount;
}

Handle<Adapter_t> VulkanResourceManager::getAdapter(const Handle<Instance_t> &instance, uint32_t index)
{
    // TODO: This is not ideal, we should only do this once rather than deviceCount times
    VkInstance vkInstance = *m_instances.get(instance);
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
    auto h = m_adapters.emplace(devices.at(index));
    return h;
}

// Handle<Adapter_t> VulkanResourceManager::createAdapter(
//         const Handle<Instance_t> &instance,
//         const AdapterOptions &options)
// {
//     return {};
// }

// void VulkanResourceManager::deleteAdapter(Handle<Adapter_t> handle)
// {
// }

} // namespace Gpu
