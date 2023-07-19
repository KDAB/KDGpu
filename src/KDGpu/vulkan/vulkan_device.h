/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_device.h>
#include <KDGpu/vulkan/vulkan_framebuffer.h>
#include <KDGpu/vulkan/vulkan_render_pass.h>

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <unordered_map>

namespace KDGpu {

class VulkanResourceManager;

struct Adapter_t;

/**
 * @brief VulkanDevice
 * \ingroup vulkan
 *
 */
struct KDGPU_EXPORT VulkanDevice : public ApiDevice {
    explicit VulkanDevice(VkDevice _device,
                          VulkanResourceManager *_vulkanResourceManager,
                          const Handle<Adapter_t> &_adapterHandle,
                          bool _isOwned = true) noexcept;

    // Non Copyable
    VulkanDevice(VulkanDevice &) noexcept = delete;
    VulkanDevice &operator=(VulkanDevice &) noexcept = delete;

    // Movable
    VulkanDevice(VulkanDevice &&) noexcept = default;
    VulkanDevice &operator=(VulkanDevice &&) noexcept = default;

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

#if defined(VK_KHR_synchronization2)
    PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2{ nullptr };
#endif
    bool isOwned{ true };
};

} // namespace KDGpu
