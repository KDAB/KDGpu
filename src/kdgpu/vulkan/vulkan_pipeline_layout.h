#pragma once

#include <kdgpu/api/api_pipeline_layout.h>

#include <kdgpu/handle.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

struct VulkanPipelineLayout : public ApiPipelineLayout {
    explicit VulkanPipelineLayout(VkPipelineLayout _pipelineLayout,
                                  std::vector<VkDescriptorSetLayout> &&_descriptorSetLayouts,
                                  VulkanResourceManager *_vulkanResourceManager,
                                  const Handle<Device_t> &_deviceHandle);

    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
