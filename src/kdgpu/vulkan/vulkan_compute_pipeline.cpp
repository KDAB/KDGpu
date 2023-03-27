#include "vulkan_compute_pipeline.h"

namespace KDGpu {

VulkanComputePipeline::VulkanComputePipeline(VkPipeline _pipeline,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle,
                                             const Handle<PipelineLayout_t> &_pipelineLayoutHandle)
    : ApiComputePipeline()
    , pipeline(_pipeline)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , pipelineLayoutHandle(_pipelineLayoutHandle)
{
}

} // namespace KDGpu
