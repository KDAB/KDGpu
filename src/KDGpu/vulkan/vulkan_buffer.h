#pragma once

#include <KDGpu/api/api_buffer.h>

#include <KDGpu/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct VulkanBuffer : public ApiBuffer {
    explicit VulkanBuffer(VkBuffer _buffer,
                          VmaAllocation _allocation,
                          VulkanResourceManager *_vulkanResourceManager,
                          const Handle<Device_t> &_deviceHandle);

    void *map() final;
    void unmap() final;

    VkBuffer buffer{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    void *mapped{ nullptr };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
