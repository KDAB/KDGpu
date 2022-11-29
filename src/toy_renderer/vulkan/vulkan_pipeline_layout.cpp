#include "vulkan_pipeline_layout.h"

namespace ToyRenderer {

VulkanPipelineLayout::VulkanPipelineLayout(VkPipelineLayout _pipelineLayout,
                                           std::vector<VkDescriptorSetLayout> &&_descriptorSetLayouts,
                                           VulkanResourceManager *_vulkanResourceManager,
                                           const Handle<Device_t> &_deviceHandle)
    : ApiPipelineLayout()
    , pipelineLayout(_pipelineLayout)
    , descriptorSetLayouts(_descriptorSetLayouts)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
