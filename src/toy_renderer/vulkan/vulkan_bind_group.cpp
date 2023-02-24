#include "vulkan_bind_group.h"

namespace ToyRenderer {

VulkanBindGroup::VulkanBindGroup(VkDescriptorSet _descriptorSet,
                                 VulkanResourceManager *_vulkanResourceManager,
                                 const Handle<Device_t> &_deviceHandle)
    : descriptorSet(_descriptorSet)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
