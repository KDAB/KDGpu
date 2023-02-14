#include "vulkan_gpu_semaphore.h"

namespace ToyRenderer {

VulkanGpuSemaphore::VulkanGpuSemaphore(VkSemaphore _semaphore,
                                       VulkanResourceManager *_vulkanResourceManager,
                                       const Handle<Device_t> &_deviceHandle)
    : ApiGpuSemaphore()
    , semaphore(_semaphore)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
{
}

} // namespace ToyRenderer
