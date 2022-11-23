#include "vulkan_buffer.h"

namespace ToyRenderer {

VulkanBuffer::VulkanBuffer(VkBuffer _buffer,
                           VmaAllocation _allocation,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle)
    : buffer(_buffer)
    , allocation(_allocation)
    , vulkanResourceManager(_vulkanResourceManager)
    , device(_device)
{
}

} // namespace ToyRenderer
