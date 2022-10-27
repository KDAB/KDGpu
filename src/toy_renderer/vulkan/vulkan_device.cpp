#include "vulkan_device.h"

namespace ToyRenderer {

VulkanDevice::VulkanDevice(VkDevice _device)
    : ApiDevice()
    , device(_device)
{
}

} // namespace ToyRenderer
