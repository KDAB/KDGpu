#pragma once

#include <toy_renderer/api/api_device.h>
#include <toy_renderer/vulkan/vulkan_framebuffer.h>
#include <toy_renderer/vulkan/vulkan_render_pass.h>

#include <toy_renderer/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <unordered_map>

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

    void waitUntilIdle() final;

    VkDevice device{ VK_NULL_HANDLE };

    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Adapter_t> adapterHandle;
    VmaAllocator allocator{ VK_NULL_HANDLE };
    std::vector<QueueDescription> queueDescriptions;
    std::vector<VkCommandPool> commandPools; // Indexed by queue type (family)
    VkDescriptorPool descriptorSetPool{ VK_NULL_HANDLE };
    std::unordered_map<VulkanRenderPassKey, Handle<RenderPass_t>> renderPasses;
    std::unordered_map<VulkanFramebufferKey, Handle<Framebuffer_t>> framebuffers;
};

} // namespace ToyRenderer
