#include "vulkan_queue.h"

namespace ToyRenderer {

VulkanQueue::VulkanQueue(VkQueue _queue)
    : ApiQueue()
    , queue(_queue)
{
}

void VulkanQueue::submit()
{
}

} // namespace ToyRenderer
