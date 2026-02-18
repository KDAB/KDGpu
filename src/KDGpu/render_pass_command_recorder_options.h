/*
This file is part of KDGpu.

SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: MIT

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/render_pass_options.h>

#include <optional>
#include <vector>

namespace KDGpu {

struct TextureView_t;
struct RenderPass_t;

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

/*!
    \brief Color attachment configuration for render passes
    \ingroup public
    \headerfile render_pass_command_recorder_options.h <KDGpu/render_pass_command_recorder_options.h>

    Specifies how a color attachment is used during rendering, including load/store operations
    and image layout transitions. In Vulkan, this maps to VkRenderingAttachmentInfo.

    \sa RenderPassCommandRecorderWithDynamicRenderingOptions, DepthStencilAttachment
 */
struct ColorAttachment {
    RequiredHandle<TextureView_t> view; ///< The texture view to render to
    OptionalHandle<TextureView_t> resolveView; ///< Optional resolve target for MSAA (leave empty for non-MSAA)
    AttachmentLoadOperation loadOperation{ AttachmentLoadOperation::Clear }; ///< How to handle existing data (Clear, Load, or DontCare)
    AttachmentStoreOperation storeOperation{ AttachmentStoreOperation::Store }; ///< Whether to store results (Store or DontCare)
    ColorClearValue clearValue; ///< Clear color if loadOperation is Clear
    TextureLayout initialLayout{ TextureLayout::Undefined }; ///< Layout before render pass (Undefined if discarding contents)
    TextureLayout layout{ TextureLayout::ColorAttachmentOptimal }; ///< Layout during rendering
    TextureLayout finalLayout{ TextureLayout::ColorAttachmentOptimal }; ///< Layout after render pass (e.g., PresentSrc for swapchain)
};

/*!
    \brief Depth/stencil attachment configuration for render passes
    \ingroup public
    \headerfile render_pass_command_recorder_options.h <KDGpu/render_pass_command_recorder_options.h>

    Configures depth and stencil testing during rendering. Depth and stencil aspects
    can have independent load/store operations. In Vulkan, this maps to VkRenderingAttachmentInfo
    for depth/stencil attachments.

    \sa RenderPassCommandRecorderWithDynamicRenderingOptions, ColorAttachment
*/
struct DepthStencilAttachment {
    OptionalHandle<TextureView_t> view; ///< The depth/stencil texture view (leave empty if not using depth/stencil)
    OptionalHandle<TextureView_t> resolveView; ///< Optional resolve target for MSAA depth/stencil
    AttachmentLoadOperation depthLoadOperation{ AttachmentLoadOperation::Clear }; ///< Depth load operation
    AttachmentStoreOperation depthStoreOperation{ AttachmentStoreOperation::Store }; ///< Depth store operation
    float depthClearValue{ 1.0f }; ///< Depth clear value (1.0 = far plane in reverse-Z, 0.0 in standard)
    ResolveModeFlagBits depthResolveMode{ ResolveModeFlagBits::Average }; ///< How to resolve MSAA depth
    AttachmentLoadOperation stencilLoadOperation{ AttachmentLoadOperation::Clear }; ///< Stencil load operation
    AttachmentStoreOperation stencilStoreOperation{ AttachmentStoreOperation::Store }; ///< Stencil store operation
    uint32_t stencilClearValue{ 0 }; ///< Stencil clear value
    ResolveModeFlagBits stencilResolveMode{ ResolveModeFlagBits::None }; ///< How to resolve MSAA stencil
    TextureLayout initialLayout{ TextureLayout::Undefined }; ///< Layout before render pass
    TextureLayout layout{ TextureLayout::DepthStencilAttachmentOptimal }; ///< Layout during rendering
    TextureLayout finalLayout{ TextureLayout::DepthStencilAttachmentOptimal }; ///< Layout after render pass
};

/*!
    \brief Legacy render pass options (deprecated in favor of dynamic rendering)
    \ingroup public
    \headerfile render_pass_command_recorder_options.h <KDGpu/render_pass_command_recorder_options.h>

    This struct is maintained for compatibility but RenderPassCommandRecorderWithDynamicRenderingOptions
    is preferred for new code on desktop platforms. It uses dynamic rendering which was promoted to Vulkan 1.3 core.

    \deprecated Use RenderPassCommandRecorderWithDynamicRenderingOptions for new code
    \sa RenderPassCommandRecorderWithDynamicRenderingOptions
*/
struct RenderPassCommandRecorderOptions {
    std::vector<ColorAttachment> colorAttachments; ///< Color render targets
    DepthStencilAttachment depthStencilAttachment; ///< Depth/stencil target

    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit }; ///< Sample count for MSAA
    uint32_t viewCount{ 1 }; ///< Number of views for multiview rendering (e.g., 2 for VR stereo)
    uint32_t framebufferWidth{ 0 }; ///< Framebuffer width (0 = infer from first attachment)
    uint32_t framebufferHeight{ 0 }; ///< Framebuffer height (0 = infer from first attachment)
    uint32_t framebufferArrayLayers{ 0 }; ///< Array layers (0 = infer from first attachment)
};

/*!
    \brief Attachment configuration for render passes
    \ingroup public
    \headerfile render_pass_command_recorder_options.h <KDGpu/render_pass_command_recorder_options.h>
*/
struct Attachment {
    RequiredHandle<TextureView_t> view;
    OptionalHandle<TextureView_t> resolveView;

    // only set for color
    struct ColorOperations {
        ColorClearValue clearValue;
        TextureLayout layout{ TextureLayout::ColorAttachmentOptimal };
    };

    // only set for depth/stencil
    struct DepthStencilOperations {
        DepthStencilClearValue clearValue;
        TextureLayout layout{ TextureLayout::DepthStencilAttachmentOptimal };
    };

    // set only one of the two
    std::optional<ColorOperations> color;
    std::optional<DepthStencilOperations> depth;
};

/*!
    \brief Render pass options using explicit VkRenderPass objects (legacy approach)
    \ingroup public
    \headerfile render_pass_command_recorder_options.h <KDGpu/render_pass_command_recorder_options.h>

    This option requires a pre-created RenderPass object with subpass dependencies.
    The VkRenderPass/VkFramebuffer model has been superseded by dynamic rendering in Vulkan 1.3.

    \note VkRenderPass and subpasses were deprecated in Vulkan 1.3 in favor of dynamic rendering
    (VK_KHR_dynamic_rendering, now core). For new applications, prefer
    RenderPassCommandRecorderWithDynamicRenderingOptions which offers more flexibility
    and simpler API usage.

    In Vulkan terms, this maps to vkCmdBeginRenderPass() with a VkRenderPass and VkFramebuffer.

    \deprecated Use RenderPassCommandRecorderWithDynamicRenderingOptions for new code
    \sa RenderPassCommandRecorderWithDynamicRenderingOptions, RenderPass
*/
struct RenderPassCommandRecorderWithRenderPassOptions {
    RequiredHandle<RenderPass_t> renderPass; ///< Pre-created RenderPass defining attachments and subpasses
    std::vector<Attachment> attachments; ///< Attachments to bind to the render pass
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit }; ///< MSAA sample count
    uint32_t viewCount{ 1 }; ///< Number of views for multiview
    uint32_t framebufferWidth{ 0 }; ///< Framebuffer width
    uint32_t framebufferHeight{ 0 }; ///< Framebuffer height
    uint32_t framebufferArrayLayers{ 0 }; ///< Framebuffer array layers
};

/*!
    \brief Modern render pass options using dynamic rendering (recommended)
    \ingroup public
    \headerfile render_pass_command_recorder_options.h <KDGpu/render_pass_command_recorder_options.h>

    Dynamic rendering allows you to specify render targets directly when beginning a render pass,
    without pre-creating VkRenderPass and VkFramebuffer objects. This approach was introduced
    as VK_KHR_dynamic_rendering and promoted to Vulkan 1.3 core, superseding the legacy
    VkRenderPass/VkFramebuffer model.

    \section renderpass_dynamic_benefits Benefits of Dynamic Rendering

    - **Simpler API**: No need to create RenderPass objects upfront
    - **More flexible**: Easily change attachments between frames
    - **Better performance**: Eliminates RenderPass compatibility checks
    - **Modern standard**: Vulkan 1.3 core feature, replacing legacy render passes

    \section renderpass_dynamic_usage Usage Example

    \snippet kdgpu_doc_snippets.cpp renderpass_dynamic_rendering_basic

    \section renderpass_dynamic_msaa MSAA with Automatic Resolve

    \snippet kdgpu_doc_snippets.cpp renderpass_msaa_resolve

    \section renderpass_dynamic_multiview Multi-View Rendering

    \snippet kdgpu_doc_snippets.cpp renderpass_multiview

    \section renderpass_dynamic_vulkan Vulkan Mapping

    In Vulkan, this maps to vkCmdBeginRendering() from VK_KHR_dynamic_rendering (Vulkan 1.3 core).

    \sa ColorAttachment, DepthStencilAttachment, RenderPassCommandRecorder
    \sa CommandRecorder::beginRenderPass()
 */
struct RenderPassCommandRecorderWithDynamicRenderingOptions {
    std::vector<ColorAttachment> colorAttachments; ///< Color render targets (can be empty for depth-only passes)
    DepthStencilAttachment depthStencilAttachment; ///< Depth/stencil target (optional)
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit }; ///< MSAA sample count
    uint32_t viewCount{ 1 }; ///< Number of views for multiview rendering (2 for stereo VR)
    uint32_t framebufferWidth{ 0 }; ///< Render area width (0 = infer from first attachment)
    uint32_t framebufferHeight{ 0 }; ///< Render area height (0 = infer from first attachment)
    uint32_t framebufferArrayLayers{ 0 }; ///< Array layers (0 = infer from first attachment)
};

struct DebugLabelOptions {
    std::string_view label;
    float color[4];
};

} // namespace KDGpu
