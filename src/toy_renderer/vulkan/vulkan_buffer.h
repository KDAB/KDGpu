#pragma once

#include <toy_renderer/api/api_buffer.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanBuffer : public ApiBuffer {
    VkBuffer buffer;
};

} // namespace ToyRenderer
