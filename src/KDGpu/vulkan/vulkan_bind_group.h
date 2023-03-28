#pragma once

#include <KDGpu/api/api_bind_group.h>
#include <KDGpu/handle.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct Device_t;

struct VulkanBindGroup : public ApiBindGroup {
    explicit VulkanBindGroup(VkDescriptorSet _descriptorSet,
                             VkDescriptorPool _descriptorPool,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle);

    void update(const BindGroupEntry &entry) final;

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
