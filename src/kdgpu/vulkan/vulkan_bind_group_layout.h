#pragma once

#include <kdgpu/api/api_bind_group_layout.h>
#include <kdgpu/handle.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;

struct VulkanBindGroupLayout : public ApiBindGroupLayout {
    explicit VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout,
                                   const Handle<Device_t> &_deviceHandle);

    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
