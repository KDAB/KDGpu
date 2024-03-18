/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_render_pass.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>
#include <KDGpu/utils/hash_utils.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;
struct RenderPassCommandRecorderOptions;
struct ColorAttachment;
struct DepthStencilAttachment;
struct Device_t;
struct KDGPU_EXPORT VulkanRenderPassKeyColorAttachment {
    explicit VulkanRenderPassKeyColorAttachment(const ColorAttachment &attachment,
                                                KDGpu::Format viewFormat,
                                                KDGpu::Format resolveViewFormat);

    bool operator==(const VulkanRenderPassKeyColorAttachment &other) const noexcept = default;
    bool operator!=(const VulkanRenderPassKeyColorAttachment &other) const noexcept = default;

    uint64_t hash{ 0 };
};

struct KDGPU_EXPORT VulkanRenderPassKeyDepthStencilAttachment {
    explicit VulkanRenderPassKeyDepthStencilAttachment(const DepthStencilAttachment &attachment,
                                                       KDGpu::Format viewFormat,
                                                       KDGpu::Format resolveViewFormat);

    bool operator==(const VulkanRenderPassKeyDepthStencilAttachment &other) const noexcept = default;
    bool operator!=(const VulkanRenderPassKeyDepthStencilAttachment &other) const noexcept = default;

    uint64_t hash{ 0 };
};
struct KDGPU_EXPORT VulkanRenderPassKey {
    explicit VulkanRenderPassKey(const RenderPassCommandRecorderOptions &options,
                                 VulkanResourceManager *resourceManager);

    bool operator==(const VulkanRenderPassKey &other) const noexcept = default;
    bool operator!=(const VulkanRenderPassKey &other) const noexcept = default;

    uint64_t hash{ 0 };
};

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
