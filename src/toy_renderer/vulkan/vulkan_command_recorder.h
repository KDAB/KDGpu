#pragma once

#include <toy_renderer/api/api_command_recorder.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;
struct Buffer_t;

struct VulkanCommandRecorder : public ApiCommandRecorder {
    explicit VulkanCommandRecorder(VkCommandPool _commandPool,
                                   VkCommandBuffer _commandBuffer,
                                   const Handle<CommandBuffer_t> _commandBufferHandle,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle);

    Handle<RenderPassCommandRecorder_t> beginRenderPass(const RenderPassCommandRecorderOptions &options) final;
    void copyBuffer(const Handle<Buffer_t> &src, const Handle<Buffer_t> &dst, size_t byteSize) final;
    Handle<CommandBuffer_t> finish() final;

    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    Handle<CommandBuffer_t> commandBufferHandle;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
