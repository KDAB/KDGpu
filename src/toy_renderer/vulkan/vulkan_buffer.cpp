#include "vulkan_buffer.h"

namespace ToyRenderer {

VulkanBuffer::VulkanBuffer(VkBuffer _buffer,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle)
    : buffer(_buffer)
    , vulkanResourceManager(_vulkanResourceManager)
    , device(_device)
{
}

} // namespace ToyRenderer
