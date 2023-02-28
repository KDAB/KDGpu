#pragma once

#include <toy_renderer/api/api_bind_group_layout.h>
#include <toy_renderer/handle.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;
struct Device_t;

struct VulkanBindGroupLayout : public ApiBindGroupLayout {
    explicit VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout);

    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
