/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGpu/vulkan/vulkan_resource_manager.h>
#include <KDGpu/render_pass_command_recorder_options.h>
#include "vulkan_render_pass.h"

namespace KDGpu {

VulkanRenderPass::VulkanRenderPass(VkRenderPass _renderPass,
                                   VulkanResourceManager *_vulkanResourceManager,
                                   Handle<Device_t> _deviceHandle)
    : ApiRenderPass()
    , renderPass(_renderPass)
    , vulkanResourceManager(_vulkanResourceManager)
    , deviceHandle(_deviceHandle)
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
        KDGpu::hash_combine(hash, colorAttachmentKey.hash);
    }

    if (options.depthStencilAttachment.view.isValid()) {
        const VulkanRenderPassKeyDepthStencilAttachment depthAttachmentKey(options.depthStencilAttachment,
                                                                           resourceManager->formatFromTextureView(options.depthStencilAttachment.view),
                                                                           resourceManager->formatFromTextureView(options.depthStencilAttachment.resolveView));
        KDGpu::hash_combine(hash, depthAttachmentKey.hash);
    }
    KDGpu::hash_combine(hash, options.samples);
    KDGpu::hash_combine(hash, options.viewCount);
}

VulkanRenderPassKeyDepthStencilAttachment::VulkanRenderPassKeyDepthStencilAttachment(const DepthStencilAttachment &attachment, KDGpu::Format viewFormat, KDGpu::Format resolveViewFormat)
{
    KDGpu::hash_combine(hash, viewFormat);
    KDGpu::hash_combine(hash, resolveViewFormat);
    KDGpu::hash_combine(hash, attachment.depthLoadOperation);
    KDGpu::hash_combine(hash, attachment.depthStoreOperation);
    KDGpu::hash_combine(hash, attachment.stencilLoadOperation);
    KDGpu::hash_combine(hash, attachment.stencilStoreOperation);
    KDGpu::hash_combine(hash, attachment.initialLayout);
    KDGpu::hash_combine(hash, attachment.finalLayout);
}

VulkanRenderPassKeyColorAttachment::VulkanRenderPassKeyColorAttachment(const ColorAttachment &attachment, KDGpu::Format viewFormat, KDGpu::Format resolveViewFormat)
{
    KDGpu::hash_combine(hash, viewFormat);
    KDGpu::hash_combine(hash, resolveViewFormat);
    KDGpu::hash_combine(hash, attachment.loadOperation);
    KDGpu::hash_combine(hash, attachment.storeOperation);
    KDGpu::hash_combine(hash, attachment.initialLayout);
    KDGpu::hash_combine(hash, attachment.finalLayout);
}

} // namespace KDGpu
