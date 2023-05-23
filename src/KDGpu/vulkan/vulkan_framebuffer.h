/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_framebuffer.h>

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
        return hash == other.hash;
    }

    bool operator!=(const VulkanAttachmentKey &other) const noexcept
    {
        return !(*this == other);
    }

    void addAttachmentView(const Handle<TextureView_t> &view)
    {
        KDGpu::hash_combine(hash, view);
    }

    uint64_t hash{ 0 };
};

struct VulkanFramebufferKey {
    Handle<RenderPass_t> renderPass;
    VulkanAttachmentKey attachmentsKey;
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint32_t layers{ 0 };

    bool operator==(const VulkanFramebufferKey &other) const noexcept
    {
        // clang-format off
        return renderPass == other.renderPass
            && attachmentsKey == other.attachmentsKey
            && width == other.width
            && height == other.height
            && layers == other.layers;
        // clang-format on
    }

    bool operator!=(const VulkanFramebufferKey &other) const noexcept
    {
        return !(*this == other);
    }
};

struct VulkanFramebuffer : public ApiFramebuffer {
    explicit VulkanFramebuffer(VkFramebuffer _framebuffer);

    VkFramebuffer framebuffer{ VK_NULL_HANDLE };
};

} // namespace KDGpu

namespace std {

template<>
struct hash<KDGpu::VulkanFramebufferKey> {
    size_t operator()(const KDGpu::VulkanFramebufferKey &value) const
    {
        uint64_t hash = 0;

        KDGpu::hash_combine(hash, value.renderPass);
        KDGpu::hash_combine(hash, value.attachmentsKey.hash);
        KDGpu::hash_combine(hash, value.width);
        KDGpu::hash_combine(hash, value.height);
        KDGpu::hash_combine(hash, value.layers);

        return hash;
    }
};

} // namespace std
