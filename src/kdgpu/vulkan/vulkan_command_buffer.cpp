#include "vulkan_command_buffer.h"

namespace KDGpu {

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer _commandBuffer,
                                         VkCommandPool _commandPool,
                                         VkCommandBufferLevel _commandLevel,
                                         VulkanResourceManager *_vulkanResourceManager,
                                         const Handle<Device_t> &_deviceHandle)
    : ApiCommandBuffer()
    , commandBuffer(_commandBuffer)
    , commandPool(_commandPool)
    , commandLevel(_commandLevel)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanCommandBuffer::begin()
{
    // Begin recording
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    if (commandLevel == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

        // TODO: We will need a way to specify the RenderPass/FrameBuffer if we want to record
        // commands from within a RenderPass.
        beginInfo.pInheritanceInfo = &inheritanceInfo;
    }

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        // TODO: Log failure to begin recording
    }
}

void VulkanCommandBuffer::finish()
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        // TODO: Log failure to end recording
    }
}

} // namespace KDGpu
