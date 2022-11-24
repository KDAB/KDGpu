#pragma once

#include <toy_renderer/api/api_pipeline_layout.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanPipelineLayout : public ApiPipelineLayout {
    explicit VulkanPipelineLayout(VkPipelineLayout _pipelineLayout,
                                  VulkanResourceManager *_vulkanResourceManager,
                                  const Handle<Device_t> &_deviceHandle);

    VkPipelineLayout pipelineLayout;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
