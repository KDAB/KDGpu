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

} // namespace KDGpu
