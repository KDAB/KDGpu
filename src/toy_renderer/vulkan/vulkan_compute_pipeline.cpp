#include "vulkan_compute_pipeline.h"

namespace ToyRenderer {

VulkanComputePipeline::VulkanComputePipeline(VkPipeline _pipeline,
                                             VkPipelineLayout _pipelineLayout,
                                             VulkanResourceManager *_vulkanResourceManager,
                                             const Handle<Device_t> &_deviceHandle)
    : ApiComputePipeline()
    , pipeline(_pipeline)
    , pipelineLayout(_pipelineLayout)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
