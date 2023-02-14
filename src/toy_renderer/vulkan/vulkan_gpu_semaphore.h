#pragma once

#include <toy_renderer/api/api_gpu_semaphore.h>

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanGpuSemaphore : public ApiGpuSemaphore {
    explicit VulkanGpuSemaphore(VkSemaphore _semaphore,
                                VulkanResourceManager *_vulkanResourceManager,
                                const Handle<Device_t> &_deviceHandle);

    VkSemaphore semaphore{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
