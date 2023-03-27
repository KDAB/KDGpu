#pragma once

#include <kdgpu/api/api_device.h>
#include <kdgpu/vulkan/vulkan_framebuffer.h>
#include <kdgpu/vulkan/vulkan_render_pass.h>

#include <kdgpu/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <unordered_map>

namespace KDGpu {

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
    std::vector<VkDescriptorPool> descriptorSetPools;
    std::unordered_map<VulkanRenderPassKey, Handle<RenderPass_t>> renderPasses;
    std::unordered_map<VulkanFramebufferKey, Handle<Framebuffer_t>> framebuffers;

    PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2{ nullptr };
};

} // namespace KDGpu
