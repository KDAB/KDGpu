#include "vulkan_command_recorder.h"
#include <toy_renderer/vulkan/vulkan_resource_manager.h>
#include <toy_renderer/vulkan/vulkan_buffer.h>
#include <toy_renderer/vulkan/vulkan_enums.h>

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

void VulkanCommandRecorder::copyBuffer(const BufferCopy &copy)
{
    VulkanBuffer *srcBuf = vulkanResourceManager->getBuffer(copy.src);
    VulkanBuffer *dstBuf = vulkanResourceManager->getBuffer(copy.dst);

    VkBufferCopy bufferCopy{};
    bufferCopy.size = copy.byteSize;
    bufferCopy.dstOffset = copy.dstOffset;
    bufferCopy.srcOffset = copy.srcOffset;

    vkCmdCopyBuffer(commandBuffer, srcBuf->buffer, dstBuf->buffer, 1, &bufferCopy);
}

void VulkanCommandRecorder::memoryBarrier(const MemoryBarrierOptions &options)
{
    std::vector<VkMemoryBarrier> memoryBarriers;

    memoryBarriers.reserve(options.memoryBarriers.size());
    for (const MemoryBarrier &b : options.memoryBarriers) {
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = accessFlagsToVkAccessFlagBits(b.srcMask);
        barrier.dstAccessMask = accessFlagsToVkAccessFlagBits(b.dstMask);
        memoryBarriers.push_back(barrier);
    }

    vkCmdPipelineBarrier(commandBuffer,
                         pipelineStageFlagsToVkPipelineStageFlagBits(options.srcStages),
                         pipelineStageFlagsToVkPipelineStageFlagBits(options.dstStages),
                         0, // None
                         memoryBarriers.size(), memoryBarriers.data(),
                         0, nullptr,
                         0, nullptr);
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
