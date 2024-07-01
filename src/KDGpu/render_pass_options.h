/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

#include <vector>
#include <optional>

namespace KDGpu {

struct SubpassDependenciesDescriptions { /* assume dependencyFlag is ignored for now */
    uint32_t srcSubpass;
    uint32_t dstSubpass;
    PipelineStageFlags srcStageMask;
    PipelineStageFlags dstStageMask;
    AccessFlags srcAccessMask;
    AccessFlags dstAccessMask;
    DependencyFlags dependencyFlags;

    int32_t viewOffsetDependency = 0; /* ignored if Multiview is not enabled */
    const uint32_t EXTERNAL_SUBPASS = ~(0u);
};

struct SubpassDescription {
    std::vector<uint32_t> inputAttachmentIndex{ 0 };
    std::vector<uint32_t> colorAttachmentIndex{ 0 };
    std::vector<uint32_t> resolveAttachmentIndex{ 0 }; /* this should be the same length as color if nonempty */
    std::vector<uint32_t> preserveAttachmentIndex{ 0 };
    std::optional<uint32_t> depthAttachmentIndex{};

    uint32_t viewMask = 0; /* ignored if Multiview is not enabled */
    std::vector<TextureAspectFlags> inputAttachmentAspects{ 0 }; /* must be filled out for multiview, override aspectEnabled in AttachmentDescription */
};

struct AttachmentDescription {
    Format format{ Format::R8G8B8A8_UNORM };
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit };
    AttachmentLoadOperation loadOp{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation storeOp{ AttachmentStoreOperation::Store };
    AttachmentLoadOperation stencilLoadOp{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation stencilstoreOp{ AttachmentStoreOperation::Store };
    TextureLayout initialLayout{ TextureLayout::Undefined };
    TextureLayout finalLayout{ TextureLayout::ColorAttachmentOptimal };
    TextureAspectFlags aspectEnabled{ TextureAspectFlagBits::None }; /* Used to override default aspect inference */
};

struct RenderPassOptions {
    std::vector<AttachmentDescription> attachments{ 0 }; /* a list of all the attacments used throughout the render pass */
    std::vector<SubpassDescription> subpassDescriptions{ 0 }; /* index in the attachments vector */
    std::vector<SubpassDependenciesDescriptions> subpassDependencies{ 0 };

    std::vector<uint32_t> correlatedViewMasks{ 0 }; /* Multiview will be enabled if this vector is set */
};

} // namespace KDGpu
