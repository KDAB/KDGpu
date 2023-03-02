#include "vulkan_fence.h"
#include <toy_renderer/vulkan/vulkan_device.h>
#include <toy_renderer/vulkan/vulkan_resource_manager.h>

namespace ToyRenderer {

VulkanFence::VulkanFence(VkFence _fence,
                         VulkanResourceManager *_vulkanResourceManager,
                         const Handle<Device_t> &_deviceHandle)
    : ApiFence()
    , fence(_fence)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanFence::wait()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vkWaitForFences(vulkanDevice->device, 1, &fence, true, std::numeric_limits<uint64_t>::max());
}

void VulkanFence::reset()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vkResetFences(vulkanDevice->device, 1, &fence);
}

} // namespace ToyRenderer
