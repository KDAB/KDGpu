#pragma once

#include <toy_renderer/api/api_command_buffer.h>
#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct Device_t;
class VulkanResourceManager;

struct VulkanCommandBuffer : public ApiCommandBuffer {
    explicit VulkanCommandBuffer(VkCommandBuffer _commandBuffer,
                                 VkCommandPool _commandPool,
                                 VkCommandBufferLevel _commandLevel,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle);

    void begin() final;
    void finish() final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VkCommandPool commandPool{ VK_NULL_HANDLE };
    VkCommandBufferLevel commandLevel{ VK_COMMAND_BUFFER_LEVEL_PRIMARY };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
