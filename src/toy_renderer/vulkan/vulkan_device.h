#pragma once

#include <toy_renderer/api/api_device.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanDevice : public ApiDevice {
    explicit VulkanDevice(VkDevice _device);

    VkDevice device{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
