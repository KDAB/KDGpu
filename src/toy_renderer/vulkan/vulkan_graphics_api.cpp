#include "vulkan_graphics_api.h"

namespace ToyRenderer {

VulkanGraphicsApi::VulkanGraphicsApi()
    : GraphicsApi()
    , m_vulkanResourceManager{ std::make_unique<VulkanResourceManager>() }
{
    m_resourceManager = m_vulkanResourceManager.get();
}

VulkanGraphicsApi::~VulkanGraphicsApi()
{
}

std::vector<Handle<Adapter_t>> VulkanGraphicsApi::queryAdapters(const Handle<Instance_t> &instanceHandle)
{
    // Query the physical devices from the instance
    VkInstance vkInstance = *m_vulkanResourceManager->getInstance(instanceHandle);

    uint32_t adapterCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &adapterCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(adapterCount);
    vkEnumeratePhysicalDevices(vkInstance, &adapterCount, physicalDevices.data());

    // Store the resulting physical devices in the resource manager so that
    // the Adapters can access them later, and create the Adapters.
    std::vector<Handle<Adapter_t>> adapterHandles;
    adapterHandles.reserve(adapterCount);
    for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex)
        adapterHandles.emplace_back(m_vulkanResourceManager->insertAdapter(physicalDevices[adapterIndex]));

    return adapterHandles;
}

} // namespace ToyRenderer
