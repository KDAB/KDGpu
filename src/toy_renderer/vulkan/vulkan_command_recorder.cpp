#include "vulkan_command_recorder.h"

namespace ToyRenderer {

VulkanCommandRecorder::VulkanCommandRecorder(VkCommandPool _commandPool,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle)
    : ApiCommandRecorder()
    , commandPool(_commandPool)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

Handle<RenderPassCommandRecorder_t> VulkanCommandRecorder::beginRenderPass(const RenderPassOptions &options)
{
    // TODO;: Implement me!
    return Handle<RenderPassCommandRecorder_t>();
}

Handle<CommandBuffer_t> VulkanCommandRecorder::finish()
{
    // TODO;: Implement me!
    return Handle<CommandBuffer_t>();
}

} // namespace ToyRenderer
