#include "vulkan_graphics_pipeline.h"

namespace ToyRenderer {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPipeline _pipeline,
                                               VkRenderPass _renderPass,
                                               VkPipelineLayout _pipelineLayout,
                                               VulkanResourceManager *_vulkanResourceManager,
                                               const Handle<Device_t> &_deviceHandle)
    : ApiGraphicsPipeline()
    , pipeline(_pipeline)
    , renderPass(_renderPass)
    , pipelineLayout(_pipelineLayout)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
