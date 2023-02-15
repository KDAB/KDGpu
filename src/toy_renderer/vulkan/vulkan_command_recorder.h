#pragma once

#include <toy_renderer/api/api_command_recorder.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanCommandRecorder : public ApiCommandRecorder {
    explicit VulkanCommandRecorder(VkCommandPool _commandPool,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle);

    Handle<RenderPassCommandRecorder_t> beginRenderPass(const RenderPassOptions &options) final;
    Handle<CommandBuffer_t> finish() final;

    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
