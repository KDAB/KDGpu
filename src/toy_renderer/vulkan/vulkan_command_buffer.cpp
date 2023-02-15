#include "vulkan_command_buffer.h"

namespace ToyRenderer {

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer _commandBuffer)
    : ApiCommandBuffer()
    , commandBuffer(_commandBuffer)
{
}

} // namespace ToyRenderer
