#pragma once

#include <KDGpu/api/api_gpu_semaphore.h>

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

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

} // namespace KDGpu
