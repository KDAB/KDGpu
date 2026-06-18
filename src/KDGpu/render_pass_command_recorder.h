/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/render_pass_command_recorder_base.h>

namespace KDGpu {

struct Device_t;
struct RenderPassCommandRecorder_t;
struct VulkanRenderPassCommandRecorder;

/*!
    \class RenderPassCommandRecorder
    \brief Records rendering commands using legacy VkRenderPass (deprecated in Vulkan 1.3)
    \ingroup public
    \headerfile render_pass_command_recorder.h <KDGpu/render_pass_command_recorder.h>

    \deprecated Use DynamicRenderPassCommandRecorder for new code. The VkRenderPass/VkFramebuffer model
    has been marked as deprecated in Vulkan 1.3 in favor of dynamic rendering (VK_KHR_dynamic_rendering).

    RenderPassCommandRecorder is used to record GPU drawing commands using the legacy VkRenderPass model.
    It cannot be created directly: instances are obtained by calling
    CommandRecorder::beginRenderPass() with RenderPassCommandRecorderOptions or
    RenderPassCommandRecorderWithRenderPassOptions.

    ## Overview

    A render pass defines:
    - Color attachments (render targets)
    - Optional depth/stencil attachment
    - Load/store operations for each attachment
    - MSAA configuration and resolve targets
    - Multi-view configuration for VR/stereo rendering
    - Subpass definitions (advanced)

    ## Creation

    \note RenderPassCommandRecorder cannot be instantiated directly. It must be created via
    CommandRecorder::beginRenderPass() using one of two option structs:
    - RenderPassCommandRecorderOptions (implicit VkRenderPass creation for simple cases)
    - RenderPassCommandRecorderWithRenderPassOptions (explicit pre-created VkRenderPass, useful for subpasses)

    ## Usage

    <b>Basic usage (implicit RenderPass):</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_legacy_basic

    <b>With explicit RenderPass (subpasses):</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_legacy_explicit

    <b>Dynamic Pipeline State:</b>

    Before drawing, you can configure the dynamic states of the pipeline:

    \snippet kdgpu_doc_snippets.cpp renderpass_viewport_scissor

    <b>Drawing:</b>

    Multiple draw command variants are available:

    \snippet kdgpu_doc_snippets.cpp renderpass_draw_variants

    <b>Push Constants:</b>

    Small amounts of data can be uploaded efficiently via push constants:

    \snippet kdgpu_doc_snippets.cpp renderpass_push_constants

    ## Advanced Features (Legacy Only)

    <b>Subpass Navigation:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_nextsubpass

    <b>Input Attachment Mapping:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_input_attachments

    ## Lifecycle

    1. Create via CommandRecorder::beginRenderPass()
    2. Set pipeline state (pipeline, vertex buffers, bind groups, viewport, etc.)
    3. Issue draw commands
    4. Call nextSubpass() if using multiple subpasses
    5. Call end() to finish the render pass
    6. The RenderPassCommandRecorder is no longer valid after end()

    ## Vulkan mapping:
    - RenderPassCommandRecorder::setPipeline() → vkCmdBindPipeline()
    - RenderPassCommandRecorder::setVertexBuffer() → vkCmdBindVertexBuffers()
    - RenderPassCommandRecorder::setIndexBuffer() → vkCmdBindIndexBuffer()
    - RenderPassCommandRecorder::setBindGroup() → vkCmdBindDescriptorSets()
    - RenderPassCommandRecorder::setViewport() → vkCmdSetViewport()
    - RenderPassCommandRecorder::setScissor() → vkCmdSetScissor()
    - RenderPassCommandRecorder::draw() → vkCmdDraw()
    - RenderPassCommandRecorder::drawIndexed() → vkCmdDrawIndexed()
    - RenderPassCommandRecorder::drawIndirect() → vkCmdDrawIndirect()
    - RenderPassCommandRecorder::pushConstant() → vkCmdPushConstants()
    - RenderPassCommandRecorder::nextSubpass() → vkCmdNextSubpass()
    - RenderPassCommandRecorder::end() → vkCmdEndRenderPass()

    ## See also:
    \sa DynamicRenderPassCommandRecorder (recommended modern API on desktop)
    \sa CommandRecorder, RenderPassCommandRecorderOptions, RenderPassCommandRecorderWithRenderPassOptions
    \sa ColorAttachment, DepthStencilAttachment, RenderPass
*/
class KDGPU_EXPORT RenderPassCommandRecorder : public RenderPassCommandRecorderBase
{

public:
    ~RenderPassCommandRecorder();

    RenderPassCommandRecorder(RenderPassCommandRecorder &&) noexcept;
    RenderPassCommandRecorder &operator=(RenderPassCommandRecorder &&) noexcept;

    RenderPassCommandRecorder(const RenderPassCommandRecorder &) = delete;
    RenderPassCommandRecorder &operator=(const RenderPassCommandRecorder &) = delete;

    const Handle<RenderPassCommandRecorder_t> &handle() const noexcept { return m_renderPassCommandRecorder; }
    bool isValid() const noexcept { return m_renderPassCommandRecorder.isValid(); }

    operator Handle<RenderPassCommandRecorder_t>() const noexcept { return m_renderPassCommandRecorder; }

    /*!
        \brief Advances to the next subpass

        Only valid when using RenderPassCommandRecorderWithRenderPassOptions.
        Not applicable to dynamic rendering.

        \deprecated Subpasses are part of the legacy VkRenderPass model

        Vulkan: vkCmdNextSubpass()
    */
    void nextSubpass();

private:
    explicit RenderPassCommandRecorder(GraphicsApi *api,
                                       const Handle<Device_t> &device,
                                       const Handle<RenderPassCommandRecorder_t> &renderPassCommandRecorder);

    VulkanRenderPassCommandRecorderBase *backend() const override;

    Handle<RenderPassCommandRecorder_t> m_renderPassCommandRecorder;

    friend class CommandRecorder;
};

} // namespace KDGpu
