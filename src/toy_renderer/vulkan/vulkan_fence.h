#pragma once

#include <toy_renderer/api/api_fence.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanFence : public ApiFence {
    explicit VulkanFence(VkFence _fence,
                         VulkanResourceManager *_vulkanResourceManager,
                         const Handle<Device_t> &_deviceHandle);

    VkFence fence{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;

    void wait() final;
    void reset() final;
};

} // namespace ToyRenderer
