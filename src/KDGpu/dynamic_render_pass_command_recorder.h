/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/handle.h>
#include <KDGpu/graphics_api.h>
#include <KDGpu/render_pass_command_recorder_base.h>

#include <span>
#include <optional>

namespace KDGpu {

struct Device_t;
struct DynamicRenderPassCommandRecorder_t;
struct VulkanDynamicRenderPassCommandRecorder;

/*!
    \class DynamicRenderPassCommandRecorder
    \brief Records rendering commands using modern dynamic rendering (Vulkan 1.3 core)
    \ingroup public
    \headerfile dynamic_render_pass_command_recorder.h <KDGpu/dynamic_render_pass_command_recorder.h>

    DynamicRenderPassCommandRecorder is the recommended modern API for recording GPU drawing commands
    that render to one or more attachments (textures). It uses VK_KHR_dynamic_rendering which was
    promoted to Vulkan 1.3 core.

    It cannot be created directly: instances are obtained by calling
    CommandRecorder::beginRenderPass() with RenderPassCommandRecorderWithDynamicRenderingOptions.

    ## Overview

    Dynamic rendering allows you to specify render targets directly when beginning a render pass,
    without pre-creating VkRenderPass and VkFramebuffer objects. This approach offers:

    - **Simpler API**: No need to create RenderPass objects upfront
    - **More flexible**: Easily change attachments between frames
    - **Better performance**: Eliminates RenderPass compatibility checks
    - **Modern standard**: Vulkan 1.3 core feature

    ## Creation

    \note DynamicRenderPassCommandRecorder cannot be instantiated directly. It must be created via
    CommandRecorder::beginRenderPass() using RenderPassCommandRecorderWithDynamicRenderingOptions.

    ## Usage

    <b>Basic usage:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_dynamic_rendering_basic

    <b>Dynamic Pipeline State:</b>

    Before drawing, you can configure the dynamic states of the pipeline:

    \snippet kdgpu_doc_snippets.cpp renderpass_viewport_scissor

    <b>Drawing:</b>

    Multiple draw command variants are available:

    \snippet kdgpu_doc_snippets.cpp renderpass_draw_variants

    <b>Push Constants:</b>

    Small amounts of data can be uploaded efficiently via push constants:

    \snippet kdgpu_doc_snippets.cpp renderpass_push_constants

    ## Advanced Features

    <b>Multiple Render Targets (MRT):</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_multiple_attachments

    <b>Load Operations:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_load_operations

    <b>MSAA with Automatic Resolve:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_msaa_resolve

    <b>Multi-View Rendering:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_multiview

    ## Lifecycle

    1. Create via CommandRecorder::beginRenderPass() with RenderPassCommandRecorderWithDynamicRenderingOptions
    2. Set pipeline state (pipeline, vertex buffers, bind groups, viewport, etc.)
    3. Issue draw commands
    4. Call end() to finish the render pass
    5. The DynamicRenderPassCommandRecorder is no longer valid after end()

    ## Vulkan mapping:
    - DynamicRenderPassCommandRecorder::setPipeline() → vkCmdBindPipeline()
    - DynamicRenderPassCommandRecorder::setVertexBuffer() → vkCmdBindVertexBuffers()
    - DynamicRenderPassCommandRecorder::setIndexBuffer() → vkCmdBindIndexBuffer()
    - DynamicRenderPassCommandRecorder::setBindGroup() → vkCmdBindDescriptorSets()
    - DynamicRenderPassCommandRecorder::setViewport() → vkCmdSetViewport()
    - DynamicRenderPassCommandRecorder::setScissor() → vkCmdSetScissor()
    - DynamicRenderPassCommandRecorder::draw() → vkCmdDraw()
    - DynamicRenderPassCommandRecorder::drawIndexed() → vkCmdDrawIndexed()
    - DynamicRenderPassCommandRecorder::drawIndirect() → vkCmdDrawIndirect()
    - DynamicRenderPassCommandRecorder::pushConstant() → vkCmdPushConstants()
    - DynamicRenderPassCommandRecorder::setInputAttachmentMapping() → vkCmdSetRenderingFragment
    - DynamicRenderPassCommandRecorder::setOutputAttachmentMapping() → vkCmdSetRenderingFragment
    - DynamicRenderPassCommandRecorder::end() → vkCmdEndRendering()

    ## See also:
    \sa CommandRecorder, RenderPassCommandRecorderWithDynamicRenderingOptions
    \sa RenderPassCommandRecorder (legacy VkRenderPass-based API)
    \sa ColorAttachment, DepthStencilAttachment, GraphicsPipeline
*/
class KDGPU_EXPORT DynamicRenderPassCommandRecorder : public RenderPassCommandRecorderBase
{
public:
    ~DynamicRenderPassCommandRecorder();

    DynamicRenderPassCommandRecorder(DynamicRenderPassCommandRecorder &&) noexcept;
    DynamicRenderPassCommandRecorder &operator=(DynamicRenderPassCommandRecorder &&) noexcept;

    DynamicRenderPassCommandRecorder(const DynamicRenderPassCommandRecorder &) = delete;
    DynamicRenderPassCommandRecorder &operator=(const DynamicRenderPassCommandRecorder &) = delete;

    const Handle<DynamicRenderPassCommandRecorder_t> &handle() const noexcept { return m_renderPassCommandRecorder; }
    bool isValid() const noexcept { return m_renderPassCommandRecorder.isValid(); }

    operator Handle<DynamicRenderPassCommandRecorder_t>() const noexcept { return m_renderPassCommandRecorder; }

    /*!
        \brief Remaps render target attachments to shader input attachment indices (e.g RenderTarget Color[0] -> Input[2])

        \param colorAttachmentIndices Color attachment remapping
        \param depthAttachmentIndex Depth attachment remapping
        \param stencilAttachmentIndex Stencil attachment remapping

        Advanced feature for reading from render targets as input attachments.
        Used for tile-based deferred rendering on mobile GPUs.
    */
    void setInputAttachmentMapping(std::span<const std::optional<uint32_t>> colorAttachmentIndices,
                                   std::optional<uint32_t> depthAttachmentIndex,
                                   std::optional<uint32_t> stencilAttachmentIndex);

    /*!
        \brief Remaps fragment shader outputs to attachment indices (e.g RenderTarget Color[0] -> Output[2])
        \param remappedOutputs Output index remapping

        Advanced feature for custom attachment output ordering.
    */
    void setOutputAttachmentMapping(std::span<const std::optional<uint32_t>> remappedOutputs);

private:
    explicit DynamicRenderPassCommandRecorder(GraphicsApi *api,
                                              const Handle<Device_t> &device,
                                              const Handle<DynamicRenderPassCommandRecorder_t> &renderPassCommandRecorder);

    VulkanRenderPassCommandRecorderBase *backend() const override;

    Handle<DynamicRenderPassCommandRecorder_t> m_renderPassCommandRecorder;

    friend class CommandRecorder;
};

} // namespace KDGpu
