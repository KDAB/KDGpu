#include "vulkan_graphics_pipeline.h"

namespace KDGpu {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPipeline _pipeline,
                                               VkRenderPass _renderPass,
                                               VulkanResourceManager *_vulkanResourceManager,
                                               const Handle<Device_t> &_deviceHandle,
                                               const Handle<PipelineLayout_t> &_pipelineLayoutHandle)
    : ApiGraphicsPipeline()
    , pipeline(_pipeline)
    , renderPass(_renderPass)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , pipelineLayoutHandle(_pipelineLayoutHandle)
{
}

} // namespace KDGpu
