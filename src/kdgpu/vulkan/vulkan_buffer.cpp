#include "vulkan_buffer.h"

#include <kdgpu/vulkan/vulkan_device.h>
#include <kdgpu/vulkan/vulkan_resource_manager.h>

namespace KDGpu {

VulkanBuffer::VulkanBuffer(VkBuffer _buffer,
                           VmaAllocation _allocation,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle)
    : buffer(_buffer)
    , allocation(_allocation)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void *VulkanBuffer::map()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaMapMemory(vulkanDevice->allocator, allocation, &mapped);
    return mapped;
}

void VulkanBuffer::unmap()
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    vmaUnmapMemory(vulkanDevice->allocator, allocation);
    mapped = nullptr;
}

} // namespace KDGpu
