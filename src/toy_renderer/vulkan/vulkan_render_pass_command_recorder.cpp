#include "vulkan_render_pass_command_recorder.h"

namespace ToyRenderer {

VulkanRenderPassCommandRecorder::VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                                                 VulkanResourceManager *_vulkanResourceManager,
                                                                 const Handle<Device_t> &_deviceHandle)
    : ApiRenderPassCommandRecorder()
    , commandBuffer(_commandBuffer)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

void VulkanRenderPassCommandRecorder::end()
{
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace ToyRenderer
