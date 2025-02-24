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

struct AttachmentReference {
    uint32_t index;
    TextureLayout layout{ TextureLayout::MaxEnum };
};

struct SubpassDependenciesDescriptions { /* assume dependencyFlag is ignored for now */
    uint32_t srcSubpass{ ExternalSubpass };
    uint32_t dstSubpass{ ExternalSubpass };
    PipelineStageFlags srcStageMask{ PipelineStageFlagBit::TopOfPipeBit };
    PipelineStageFlags dstStageMask{ PipelineStageFlagBit::BottomOfPipeBit };
    AccessFlags srcAccessMask{ AccessFlagBit::None };
    AccessFlags dstAccessMask{ AccessFlagBit::None };
    DependencyFlags dependencyFlags{ DependencyFlagBits::ByRegion };

    int32_t viewOffsetDependency = 0; /* ignored if Multiview is not enabled */
};

struct SubpassDescription {
    std::vector<AttachmentReference> inputAttachmentReference;
    std::vector<AttachmentReference> colorAttachmentReference;
    std::vector<AttachmentReference> resolveAttachmentReference; /* this should be the same length as color if nonempty */
    std::vector<uint32_t> preserveAttachmentIndex;
    std::optional<AttachmentReference> depthAttachmentReference;
    std::optional<AttachmentReference> depthResolveAttachmentReference;

    uint32_t viewMask = 0; /* ignored if Multiview is not enabled */
    std::vector<TextureAspectFlags> inputAttachmentAspects; /* must be filled out for multiview, override aspectEnabled in AttachmentDescription */
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
    std::vector<AttachmentDescription> attachments; /* a list of all the attacments used throughout the render pass */
    std::vector<SubpassDescription> subpassDescriptions; /* index in the attachments vector */
    std::vector<SubpassDependenciesDescriptions> subpassDependencies;

    std::vector<uint32_t> correlatedViewMasks; /* Multiview will be enabled if this vector is set */
};

} // namespace KDGpu
