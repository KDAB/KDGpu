#include "vulkan_command_recorder.h"
#include <toy_renderer/vulkan/vulkan_resource_manager.h>
#include <toy_renderer/vulkan/vulkan_buffer.h>
#include <toy_renderer/vulkan/vulkan_command_buffer.h>
#include <toy_renderer/vulkan/vulkan_enums.h>

// MemoryBarrier is a define in winnt.h
#if defined(MemoryBarrier)
#undef MemoryBarrier
#endif

namespace ToyRenderer {

VulkanCommandRecorder::VulkanCommandRecorder(VkCommandPool _commandPool,
                                             const Handle<CommandBuffer_t> _commandBufferHandle,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle)
    : ApiCommandRecorder()
    , commandPool(_commandPool)
    , commandBufferHandle(_commandBufferHandle)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
    VulkanCommandBuffer *vulkanCommandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer = vulkanCommandBuffer->commandBuffer;
}

void VulkanCommandRecorder::begin()
{
    VulkanCommandBuffer *commandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer->begin();
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

// TODO: Implement an array version
void VulkanCommandRecorder::bufferMemoryBarrier(const BufferMemoryBarrierOptions &options)
{
    auto vulkanDevice = vulkanResourceManager->getDevice(deviceHandle);
    if (vulkanDevice->vkCmdPipelineBarrier2 != nullptr) {
        VkBufferMemoryBarrier2 vkBufferBarrier = {};
        vkBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        vkBufferBarrier.srcStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.srcStages);
        vkBufferBarrier.srcAccessMask = accessFlagsToVkAccessFlagBits2(options.srcMask);
        vkBufferBarrier.dstStageMask = pipelineStageFlagsToVkPipelineStageFlagBits2(options.dstStages);
        vkBufferBarrier.dstAccessMask = accessFlagsToVkAccessFlagBits2(options.dstMask);
        vkBufferBarrier.srcQueueFamilyIndex = options.srcQueueTypeIndex;
        vkBufferBarrier.dstQueueFamilyIndex = options.dstQueueTypeIndex;

        const auto vulkanBuffer = vulkanResourceManager->getBuffer(options.buffer);
        vkBufferBarrier.buffer = vulkanBuffer->buffer;
        vkBufferBarrier.offset = options.offset;
        vkBufferBarrier.size = options.size;

        VkDependencyInfo vkDependencyInfo = {};
        vkDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        vkDependencyInfo.bufferMemoryBarrierCount = 1;
        vkDependencyInfo.pBufferMemoryBarriers = &vkBufferBarrier;

        vulkanDevice->vkCmdPipelineBarrier2(commandBuffer, &vkDependencyInfo);
    } else {
        // Fallback to the Vulkan 1.0 approach
        VkBufferMemoryBarrier vkBufferBarrier = {};
        vkBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        vkBufferBarrier.srcAccessMask = accessFlagsToVkAccessFlagBits(options.srcMask);
        vkBufferBarrier.dstAccessMask = accessFlagsToVkAccessFlagBits(options.dstMask);

        const auto vulkanBuffer = vulkanResourceManager->getBuffer(options.buffer);
        vkBufferBarrier.buffer = vulkanBuffer->buffer;
        vkBufferBarrier.offset = options.offset;
        vkBufferBarrier.size = options.size;

        vkCmdPipelineBarrier(commandBuffer,
                             pipelineStageFlagsToVkPipelineStageFlagBits(options.srcStages),
                             pipelineStageFlagsToVkPipelineStageFlagBits(options.dstStages),
                             0, // None
                             0, nullptr,
                             1, &vkBufferBarrier,
                             0, nullptr);
    }
}

void VulkanCommandRecorder::executeSecondaryCommandBuffer(const Handle<CommandBuffer_t> &secondaryCommandBuffer)
{
    VulkanCommandBuffer *vulkanSecondaryCommandBuffer = vulkanResourceManager->getCommandBuffer(secondaryCommandBuffer);
    vkCmdExecuteCommands(commandBuffer, 1, &vulkanSecondaryCommandBuffer->commandBuffer);
}

Handle<CommandBuffer_t> VulkanCommandRecorder::finish()
{
    VulkanCommandBuffer *commandBuffer = vulkanResourceManager->getCommandBuffer(commandBufferHandle);
    commandBuffer->finish();
    return commandBufferHandle;
}

} // namespace ToyRenderer
