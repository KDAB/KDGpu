#include "vulkan_command_recorder.h"

namespace ToyRenderer {

VulkanCommandRecorder::VulkanCommandRecorder(VkCommandPool _commandPool,
                                             VkCommandBuffer _commandBuffer,
                                             const Handle<CommandBuffer_t> _commandBufferHandle,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle)
    : ApiCommandRecorder()
    , commandPool(_commandPool)
    , commandBuffer(_commandBuffer)
    , commandBufferHandle(_commandBufferHandle)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

Handle<RenderPassCommandRecorder_t> VulkanCommandRecorder::beginRenderPass(const RenderPassCommandRecorderOptions &options)
{
    // TODO;: Implement me!
    return Handle<RenderPassCommandRecorder_t>();
}

Handle<CommandBuffer_t> VulkanCommandRecorder::finish()
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        // TODO: Log failure to end recording
        return {};
    }
    return commandBufferHandle;
}

} // namespace ToyRenderer
