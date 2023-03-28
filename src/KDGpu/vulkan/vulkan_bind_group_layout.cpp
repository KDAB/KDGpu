#include "vulkan_bind_group_layout.h"

namespace KDGpu {

VulkanBindGroupLayout::VulkanBindGroupLayout(VkDescriptorSetLayout _descriptorSetLayout,
                                             const Handle<Device_t> &_deviceHandle)
    : descriptorSetLayout(_descriptorSetLayout)
    , deviceHandle(_deviceHandle)
{
}

} // namespace KDGpu
