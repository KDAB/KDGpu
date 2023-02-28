#include "vulkan_sampler.h"

namespace ToyRenderer {

VulkanSampler::VulkanSampler(VkSampler _sampler, const Handle<Device_t> &_deviceHandle)
    : sampler(_sampler)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
