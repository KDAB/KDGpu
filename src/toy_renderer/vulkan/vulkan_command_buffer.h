#pragma once

#include <toy_renderer/api/api_command_buffer.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanCommandBuffer : public ApiCommandBuffer {
    explicit VulkanCommandBuffer(VkCommandBuffer _commandBuffer);

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
