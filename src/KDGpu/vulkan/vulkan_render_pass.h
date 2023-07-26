/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_render_pass.h>

#include <KDGpu/handle.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDGpu/utils/hash_utils.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct VulkanRenderPassKeyColorAttachment {
    explicit VulkanRenderPassKeyColorAttachment(const ColorAttachment &attachment)
    {
        KDGpu::hash_combine(hash, attachment.loadOperation);
        KDGpu::hash_combine(hash, attachment.storeOperation);
        KDGpu::hash_combine(hash, attachment.initialLayout);
        KDGpu::hash_combine(hash, attachment.finalLayout);
    }

    bool operator==(const VulkanRenderPassKeyColorAttachment &other) const noexcept
    {
        return hash == other.hash;
    }

    bool operator!=(const VulkanRenderPassKeyColorAttachment &other) const noexcept
    {
        return !(*this == other);
    }

    uint64_t hash{ 0 };
};

struct VulkanRenderPassKeyDepthStencilAttachment {
    explicit VulkanRenderPassKeyDepthStencilAttachment(const DepthStencilAttachment &attachment)
    {
        KDGpu::hash_combine(hash, attachment.depthLoadOperation);
        KDGpu::hash_combine(hash, attachment.depthStoreOperation);
        KDGpu::hash_combine(hash, attachment.stencilLoadOperation);
        KDGpu::hash_combine(hash, attachment.stencilStoreOperation);
        KDGpu::hash_combine(hash, attachment.initialLayout);
        KDGpu::hash_combine(hash, attachment.finalLayout);
    }

    bool operator==(const VulkanRenderPassKeyDepthStencilAttachment &other) const noexcept
    {
        return hash == other.hash;
    }

    bool operator!=(const VulkanRenderPassKeyDepthStencilAttachment &other) const noexcept
    {
        return !(*this == other);
    }

    uint64_t hash{ 0 };
};

struct VulkanRenderPassKey {
    explicit VulkanRenderPassKey(const RenderPassCommandRecorderOptions &options)
    {
        const uint32_t colorAttachmentCount = static_cast<uint32_t>(options.colorAttachments.size());
        for (uint32_t i = 0; i < colorAttachmentCount; ++i) {
            const VulkanRenderPassKeyColorAttachment colorAttachmentKey{ options.colorAttachments.at(i) };
            KDGpu::hash_combine(hash, colorAttachmentKey.hash);
        }

        const VulkanRenderPassKeyDepthStencilAttachment depthAttachmentKey(options.depthStencilAttachment);
        KDGpu::hash_combine(hash, depthAttachmentKey.hash);
        KDGpu::hash_combine(hash, options.samples);
        KDGpu::hash_combine(hash, options.viewCount);
    }

    bool operator==(const VulkanRenderPassKey &other) const noexcept
    {
        return hash == other.hash;
    }

    bool operator!=(const VulkanRenderPassKey &other) const noexcept
    {
        return !(*this == other);
    }

    uint64_t hash{ 0 };
};

class VulkanResourceManager;

struct Device_t;

struct VulkanRenderPass : public ApiRenderPass {
    explicit VulkanRenderPass(VkRenderPass _renderPass,
                              VulkanResourceManager *_vulkanResourceManager,
                              Handle<Device_t> _deviceHandle);

    VkRenderPass renderPass{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu

namespace std {

template<>
struct hash<KDGpu::VulkanRenderPassKey> {
    size_t operator()(const KDGpu::VulkanRenderPassKey &key) const
    {
        return key.hash;
    }
};

} // namespace std
