/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include <KDFoundation/hashutils.h>
#include "vulkan_render_pass.h"

namespace {
template<class>
inline constexpr bool always_false = false;
}

namespace KDGpu {

VulkanRenderPass::VulkanRenderPass(VkRenderPass _renderPass,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   Handle<Device_t> _deviceHandle,
                                   const std::vector<AttachmentDescription> &_attachmentDescriptions)
    : renderPass(_renderPass)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
    , attachmentDescriptions(_attachmentDescriptions)
{
}

VulkanRenderPassKey::VulkanRenderPassKey(const RenderPassCommandRecorderOptions &options, VulkanResourceManager *resourceManager)
{
    const size_t colorAttachmentCount = options.colorAttachments.size();
    for (size_t i = 0; i < colorAttachmentCount; ++i) {
        const ColorAttachment &colorAttachment = options.colorAttachments[i];
        const VulkanRenderPassKeyColorAttachment colorAttachmentKey(
                colorAttachment,
                resourceManager->formatFromTextureView(colorAttachment.view),
                resourceManager->formatFromTextureView(colorAttachment.resolveView));
        KDFoundation::hash_combine(hash, colorAttachmentKey.hash);
    }

    if (options.depthStencilAttachment.view.isValid()) {
        const VulkanRenderPassKeyDepthStencilAttachment depthAttachmentKey(options.depthStencilAttachment,
                                                                           resourceManager->formatFromTextureView(options.depthStencilAttachment.view),
                                                                           resourceManager->formatFromTextureView(options.depthStencilAttachment.resolveView));
        KDFoundation::hash_combine(hash, depthAttachmentKey.hash);
    }
    KDFoundation::hash_combine(hash, options.samples);
    KDFoundation::hash_combine(hash, options.viewCount);
}

VulkanRenderPassKeyDepthStencilAttachment::VulkanRenderPassKeyDepthStencilAttachment(const DepthStencilAttachment &attachment, KDGpu::Format viewFormat, KDGpu::Format resolveViewFormat)
{
    KDFoundation::hash_combine(hash, viewFormat);
    KDFoundation::hash_combine(hash, resolveViewFormat);
    KDFoundation::hash_combine(hash, attachment.depthLoadOperation);
    KDFoundation::hash_combine(hash, attachment.depthStoreOperation);
    KDFoundation::hash_combine(hash, attachment.stencilLoadOperation);
    KDFoundation::hash_combine(hash, attachment.stencilStoreOperation);
    KDFoundation::hash_combine(hash, attachment.initialLayout);
    KDFoundation::hash_combine(hash, attachment.finalLayout);
}

VulkanRenderPassKeyColorAttachment::VulkanRenderPassKeyColorAttachment(const ColorAttachment &attachment, KDGpu::Format viewFormat, KDGpu::Format resolveViewFormat)
{
    KDFoundation::hash_combine(hash, viewFormat);
    KDFoundation::hash_combine(hash, resolveViewFormat);
    KDFoundation::hash_combine(hash, attachment.loadOperation);
    KDFoundation::hash_combine(hash, attachment.storeOperation);
    KDFoundation::hash_combine(hash, attachment.initialLayout);
    KDFoundation::hash_combine(hash, attachment.finalLayout);
}

} // namespace KDGpu
