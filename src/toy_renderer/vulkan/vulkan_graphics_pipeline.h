#pragma once

#include <toy_renderer/api/api_graphics_pipeline.h>

#include <toy_renderer/handle.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanGraphicsPipeline : public ApiGraphicsPipeline {
    explicit VulkanGraphicsPipeline(VkPipeline _pipeline,
                                    VkRenderPass _renderPass,
                                    VkPipelineLayout _pipelineLayout,
                                    VulkanResourceManager *_vulkanResourceManager,
                                    const Handle<Device_t> &_deviceHandle);

    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkRenderPass renderPass{ VK_NULL_HANDLE };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
