#include "vulkan_bind_group_layout.h"

namespace ToyRenderer {

VulkanBindGroupLayout::VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout)
    : descriptorSetLayout(_descriptorSetLayout)
{
}

} // namespace ToyRenderer
