#include "vulkan_pipeline_layout.h"

namespace ToyRenderer {

VulkanPipelineLayout::VulkanPipelineLayout(VkPipelineLayout _pipelineLayout,
                                           VulkanResourceManager *_vulkanResourceManager,
                                           const Handle<Device_t> &_deviceHandle)
    : ApiPipelineLayout()
    , pipelineLayout(_pipelineLayout)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
