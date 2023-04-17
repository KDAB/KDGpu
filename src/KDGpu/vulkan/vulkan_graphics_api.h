#pragma once

#include <KDGpu/graphics_api.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <memory>

namespace KDGpu {

class KDGPU_EXPORT VulkanGraphicsApi final : public GraphicsApi
{
public:
    VulkanGraphicsApi();
    ~VulkanGraphicsApi() final;

    Instance createInstanceFromExistingVkInstance(VkInstance vkInstance);
    Surface createSurfaceFromExistingVkSurface(const Handle<Instance_t> &instanceH, VkSurfaceKHR vkSurface);
    Adapter createAdapterFromExistingVkPhysicalDevice(const Handle<Instance_t> &instanceH, VkPhysicalDevice vkPhysicalDevice);
    Queue createQueueFromExistingVkQueue(VkQueue vkQueue, const QueueFlags queueFlags);
    Device createDeviceFromExisitingVkDevice(Adapter *adapter,
                                             VkDevice vkDevice,
                                             std::vector<Queue> &&queues);

private:
    std::unique_ptr<VulkanResourceManager> m_vulkanResourceManager;
};

} // namespace KDGpu
