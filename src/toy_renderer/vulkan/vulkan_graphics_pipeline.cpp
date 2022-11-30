#include "vulkan_graphics_pipeline.h"

namespace ToyRenderer {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPipeline _pipeline,
                                               VkRenderPass _renderPass,
                                               VulkanResourceManager *_vulkanResourceManager,
                                               const Handle<Device_t> &_deviceHandle)
    : ApiGraphicsPipeline()
    , pipeline(_pipeline)
    , renderPass(_renderPass)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
