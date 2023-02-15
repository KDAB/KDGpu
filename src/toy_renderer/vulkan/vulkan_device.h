#pragma once

#include <toy_renderer/api/api_device.h>

#include <toy_renderer/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Adapter_t;

struct VulkanDevice : public ApiDevice {
    explicit VulkanDevice(VkDevice _device,
                          VulkanResourceManager *_vulkanResourceManager,
                          const Handle<Adapter_t> &_adapterHandle);

    std::vector<QueueDescription> getQueues(ResourceManager *resourceManager,
                                            const std::vector<QueueRequest> &queueRequests,
                                            std::span<AdapterQueueType> queueTypes) final;

    VkDevice device{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Adapter_t> adapterHandle;
    VmaAllocator allocator{ VK_NULL_HANDLE };
    std::vector<QueueDescription> queueDescriptions;
    std::vector<VkCommandPool> commandPools; // Indexed by queue type (family)
};

} // namespace ToyRenderer
