#pragma once

#include <toy_renderer/api/api_queue.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanQueue : public ApiQueue {
    explicit VulkanQueue(VkQueue _queue);

    void submit() final;

    VkQueue queue{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
