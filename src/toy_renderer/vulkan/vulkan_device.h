#pragma once

#include <toy_renderer/api/api_device.h>

#include <vulkan/vulkan.h>

namespace ToyRenderer {

struct VulkanDevice : public ApiDevice {
    explicit VulkanDevice(VkDevice _device);

    std::vector<QueueDescription> getQueues(ResourceManager *resourceManager,
                                            const std::vector<QueueRequest> &queueRequests,
                                            std::span<AdapterQueueType> queueTypes) final;

    VkDevice device{ VK_NULL_HANDLE };
};

} // namespace ToyRenderer
