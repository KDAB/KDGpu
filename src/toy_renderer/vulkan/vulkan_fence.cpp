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

FenceStatus VulkanFence::status()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    VkResult vkResult = vkGetFenceStatus(vulkanDevice->device, fence);
    switch (vkResult) {
    case VK_SUCCESS:
        return FenceStatus::Signalled;
    case VK_NOT_READY:
        return FenceStatus::Unsignalled;
    default:
        return FenceStatus::Error;
    }
}

} // namespace ToyRenderer
