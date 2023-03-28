#pragma once

#include <kdgpu/api/api_fence.h>

#include <kdgpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

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
    FenceStatus status() final;
};

} // namespace KDGpu
