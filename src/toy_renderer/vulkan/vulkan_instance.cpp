#include "vulkan_instance.h"

#include <toy_renderer/vulkan/vulkan_adapter.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanInstance::VulkanInstance(VulkanResourceManager *_vulkanResourceManager, VkInstance _instance)
    : ApiInstance()
    , vulkanResourceManager(_vulkanResourceManager)
    , instance(_instance)
{
}

std::vector<Handle<Adapter_t>> VulkanInstance::queryAdapters()
{
    // Query the physical devices from the instance
    uint32_t adapterCount = 0;
    vkEnumeratePhysicalDevices(instance, &adapterCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(adapterCount);
    vkEnumeratePhysicalDevices(instance, &adapterCount, physicalDevices.data());

    // Store the resulting physical devices in the resource manager so that
    // the Adapters can access them later, and create the Adapters.
    std::vector<Handle<Adapter_t>> adapterHandles;
    adapterHandles.reserve(adapterCount);
    for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
        VulkanAdapter vulkanAdapter{ physicalDevices[adapterIndex] };
        adapterHandles.emplace_back(vulkanResourceManager->insertAdapter(vulkanAdapter));
    }

    return adapterHandles;
}

} // namespace ToyRenderer
