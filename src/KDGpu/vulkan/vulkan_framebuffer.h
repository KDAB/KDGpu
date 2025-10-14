/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/utils/hash_utils.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace KDGpu {

struct Framebuffer_t;
struct RenderPass_t;
struct TextureView_t;

struct VulkanAttachmentKey {

    bool operator==(const VulkanAttachmentKey &other) const noexcept
    {
        return handles == other.handles;
    }

    bool operator!=(const VulkanAttachmentKey &other) const noexcept
    {
        return !(*this == other);
    }

    void addAttachmentView(const Handle<TextureView_t> &view)
    {
        handles.push_back(view);
    }

    std::vector<KDGpu::Handle<TextureView_t>> handles;
};

struct VulkanFramebufferKey {
    Handle<RenderPass_t> renderPass;
    VulkanAttachmentKey attachmentsKey;
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint32_t layers{ 0 };
    uint32_t viewCount{ 0 };

    bool operator==(const VulkanFramebufferKey &other) const noexcept
    {
        // clang-format off
        return renderPass == other.renderPass
            && attachmentsKey == other.attachmentsKey
            && width == other.width
            && height == other.height
            && layers == other.layers
            && viewCount == other.viewCount;
        // clang-format on
    }

    bool operator!=(const VulkanFramebufferKey &other) const noexcept
    {
        return !(*this == other);
    }
};

struct VulkanFramebuffer {
    explicit VulkanFramebuffer(VkFramebuffer _framebuffer);

    inline static int DefaultScore = 5;

    VkFramebuffer framebuffer{ VK_NULL_HANDLE };
    int score{ DefaultScore };
};

} // namespace KDGpu

namespace std {

template<>
struct hash<KDGpu::VulkanAttachmentKey> {
    size_t operator()(const KDGpu::VulkanAttachmentKey &value) const
    {
        size_t hash = 0;

        for (const KDGpu::Handle<KDGpu::TextureView_t> &handle : value.handles)
            KDGpu::hash_combine(hash, handle);

        return hash;
    }
};

template<>
struct hash<KDGpu::VulkanFramebufferKey> {
    size_t operator()(const KDGpu::VulkanFramebufferKey &value) const
    {
        size_t hash = 0;

        KDGpu::hash_combine(hash, value.renderPass);
        KDGpu::hash_combine(hash, value.attachmentsKey);
        KDGpu::hash_combine(hash, value.width);
        KDGpu::hash_combine(hash, value.height);
        KDGpu::hash_combine(hash, value.layers);
        KDGpu::hash_combine(hash, value.viewCount);

        return hash;
    }
};

} // namespace std
