#pragma once

#include <toy_renderer/api/api_render_pass_command_recorder.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanRenderPassCommandRecorder : public ApiRenderPassCommandRecorder {
    explicit VulkanRenderPassCommandRecorder(VkCommandBuffer _commandBuffer,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle);

    void end() final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
