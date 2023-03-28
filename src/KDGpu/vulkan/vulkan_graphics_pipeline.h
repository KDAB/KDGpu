#pragma once

#include <KDGpu/api/api_graphics_pipeline.h>

#include <KDGpu/handle.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct PipelineLayout_t;

struct VulkanGraphicsPipeline : public ApiGraphicsPipeline {
    explicit VulkanGraphicsPipeline(VkPipeline _pipeline,
                                    VkRenderPass _renderPass,
                                    VulkanResourceManager *_vulkanResourceManager,
                                    const Handle<Device_t> &_deviceHandle,
                                    const Handle<PipelineLayout_t> &_pipelineLayoutHandle);

    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    Handle<PipelineLayout_t> pipelineLayoutHandle;
};

} // namespace KDGpu
