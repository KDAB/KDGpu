/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct TextureView_t;

// NB: The image layouts in these structs are listed as:
// * initialLayout
//      the layout of the attached image immediately before the render pass begins
//
// * layout
//      the layout into which the image will be transitioned ready for the render pass
//
// * finalLayout
//      the layout into which the image will be transitioned after the render pass is done.
//
// The default is to specify TextureLayout::Undefined for the initialLayout which means
// that the image may be in any layout but that the contents of the image can be discarded
// at the start of the render pass.
//
// If you need to preserve the contents of an image on the way in to the render pass then
// you must explicitly specify the initialLayout of the image correctly so that the driver
// can properly transition the image non-destructively.
struct ColorAttachment {
    Handle<TextureView_t> view;
    Handle<TextureView_t> resolveView;
    AttachmentLoadOperation loadOperation{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation storeOperation{ AttachmentStoreOperation::Store };
    ColorClearValue clearValue;
    TextureLayout initialLayout{ TextureLayout::Undefined };
    TextureLayout layout{ TextureLayout::ColorAttachmentOptimal };
    TextureLayout finalLayout{ TextureLayout::ColorAttachmentOptimal };
};

struct DepthStencilAttachment {
    Handle<TextureView_t> view;
    Handle<TextureView_t> resolveView;
    AttachmentLoadOperation depthLoadOperation{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation depthStoreOperation{ AttachmentStoreOperation::Store };
    float depthClearValue{ 1.0f };
    ResolveModeFlagBits depthResolveMode{ ResolveModeFlagBits::Average };
    AttachmentLoadOperation stencilLoadOperation{ AttachmentLoadOperation::Clear };
    AttachmentStoreOperation stencilStoreOperation{ AttachmentStoreOperation::Store };
    uint32_t stencilClearValue{ 0 };
    ResolveModeFlagBits stencilResolveMode{ ResolveModeFlagBits::None };
    TextureLayout initialLayout{ TextureLayout::Undefined };
    TextureLayout layout{ TextureLayout::DepthStencilAttachmentOptimal };
    TextureLayout finalLayout{ TextureLayout::DepthStencilAttachmentOptimal };
};

// TODO: Make the depthStencilAttachment optional with std::optional?
struct RenderPassCommandRecorderOptions {
    std::vector<ColorAttachment> colorAttachments;
    DepthStencilAttachment depthStencilAttachment;
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit };
    uint32_t viewCount{ 1 };
    uint32_t framebufferWidth{ 0 }; // Default to first color attachment width
    uint32_t framebufferHeight{ 0 }; // Default to first color attachment height
    uint32_t framebufferArrayLayers{ 0 }; // Default to first color attachment arrayLayer
};

} // namespace KDGpu
