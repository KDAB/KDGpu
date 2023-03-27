#pragma once

#include <kdgpu/api/api_queue.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct VulkanQueue : public ApiQueue {
    explicit VulkanQueue(VkQueue _queue,
                         VulkanResourceManager *_vulkanResourceManager);

    void waitUntilIdle() final;
    void submit(const SubmitOptions &options) final;
    std::vector<PresentResult> present(const PresentOptions &options) final;

    VkQueue queue{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
};

} // namespace KDGpu
