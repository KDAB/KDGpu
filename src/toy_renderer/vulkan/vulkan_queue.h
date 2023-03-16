#pragma once

#include <toy_renderer/api/api_queue.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct VulkanQueue : public ApiQueue {
    explicit VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager);

    void submit(const SubmitOptions &options) final;
    std::vector<PresentResult> present(const PresentOptions &options) final;

    VkQueue queue{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
};

} // namespace ToyRenderer
