#pragma once

#include <toy_renderer/api/api_buffer.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;
struct VulkanBuffer : public ApiBuffer {
    VulkanBuffer(VkBuffer _buffer,
                 VulkanResourceManager *_vulkanResourceManager,
                 const Handle<Device_t> &_deviceHandle);

    VkBuffer buffer{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
