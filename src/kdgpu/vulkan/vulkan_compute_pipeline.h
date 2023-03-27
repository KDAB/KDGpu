#pragma once

#include <kdgpu/api/api_compute_pipeline.h>
#include <kdgpu/handle.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;
struct PipelineLayout_t;

struct VulkanComputePipeline : public ApiComputePipeline {
    explicit VulkanComputePipeline(VkPipeline _pipeline,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   const Handle<Device_t> &_deviceHandle,
                                   const Handle<PipelineLayout_t> &_pipelineLayoutHandle);

    VkPipeline pipeline{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager;
    Handle<Device_t> deviceHandle;
    Handle<PipelineLayout_t> pipelineLayoutHandle;
};

} // namespace KDGpu
