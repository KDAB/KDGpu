#include "vulkan_command_recorder.h"
#include <toy_renderer/vulkan/vulkan_resource_manager.h>
#include <toy_renderer/vulkan/vulkan_buffer.h>

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

void VulkanCommandRecorder::copyBuffer(const Handle<Buffer_t> &src, const Handle<Buffer_t> &dst, size_t byteSize)
{
    VulkanBuffer *srcBuf = vulkanResourceManager->getBuffer(src);
    VulkanBuffer *dstBuf = vulkanResourceManager->getBuffer(dst);

    VkBufferCopy bufferCopy{};
    bufferCopy.size = byteSize;
    bufferCopy.dstOffset = 0;
    bufferCopy.srcOffset = 0;

    vkCmdCopyBuffer(commandBuffer, srcBuf->buffer, dstBuf->buffer, 1, &bufferCopy);
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
