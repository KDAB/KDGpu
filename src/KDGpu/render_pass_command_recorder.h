/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

#include <span>
#include <optional>

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct GraphicsPipeline_t;
struct PipelineLayout_t;
struct RenderPassCommandRecorder_t;

struct Rect2D;
struct Viewport;
struct PushConstantRange;
struct BindGroupEntry;

/*!
    \brief Parameters for non-indexed draw commands

    Used with RenderPassCommandRecorder::draw() to draw vertices directly from vertex buffers
    without using an index buffer.

    \sa RenderPassCommandRecorder::draw()
*/
struct DrawCommand {
    uint32_t vertexCount{ 0 }; ///< Number of vertices to draw
    uint32_t instanceCount{ 1 }; ///< Number of instances to draw (1 = no instancing)
    uint32_t firstVertex{ 0 }; ///< Index of first vertex to draw
    uint32_t firstInstance{ 0 }; ///< ID of first instance (used as gl_InstanceIndex base)
};

/*!
    \brief Parameters for indexed draw commands

    Most commonly used draw command type. Reads indices from the bound index buffer which
    reference vertices in vertex buffers, allowing vertex reuse and reduced memory usage.

    \sa RenderPassCommandRecorder::drawIndexed(), RenderPassCommandRecorder::setIndexBuffer()
*/
struct DrawIndexedCommand {
    uint32_t indexCount{ 0 }; ///< Number of indices to draw
    uint32_t instanceCount{ 1 }; ///< Number of instances to draw
    uint32_t firstIndex{ 0 }; ///< Index of first index to read from index buffer
    int32_t vertexOffset{ 0 }; ///< Offset added to each index value before accessing vertex buffer
    uint32_t firstInstance{ 0 }; ///< ID of first instance
};

/*!
    \brief Parameters for GPU-driven non-indexed drawing

    Draw parameters are stored in a GPU buffer, enabling the GPU to determine draw
    parameters without CPU involvement. Useful for GPU culling and LOD systems.

    \sa RenderPassCommandRecorder::drawIndirect()
*/
struct DrawIndirectCommand {
    Handle<Buffer_t> buffer; ///< Buffer containing VkDrawIndirectCommand structures
    size_t offset{ 0 }; ///< Byte offset into buffer
    uint32_t drawCount{ 0 }; ///< Number of draws to execute
    uint32_t stride{ 0 }; ///< Byte stride between consecutive draw commands
};

/*!
    \brief Parameters for GPU-driven indexed drawing

    Indexed drawing with parameters in a GPU buffer. Combines benefits of indexed
    rendering and GPU-driven draw calls.

    \sa RenderPassCommandRecorder::drawIndexedIndirect()
*/
struct DrawIndexedIndirectCommand {
    Handle<Buffer_t> buffer; ///< Buffer containing VkDrawIndexedIndirectCommand structures
    size_t offset{ 0 }; ///< Byte offset into buffer
    uint32_t drawCount{ 0 }; ///< Number of indexed draws to execute
    uint32_t stride{ 0 }; ///< Byte stride between consecutive draw commands
};

/*!
    \brief Parameters for mesh shader task dispatch

    Mesh shaders replace the traditional vertex/geometry shader stages with a programmable
    geometry pipeline. Requires VK_EXT_mesh_shader support.

    \sa RenderPassCommandRecorder::drawMeshTasks()
*/
struct DrawMeshCommand {
    uint32_t workGroupX{ 1 }; ///< X dimension of mesh shader work groups
    uint32_t workGroupY{ 1 }; ///< Y dimension of mesh shader work groups
    uint32_t workGroupZ{ 1 }; ///< Z dimension of mesh shader work groups
};

/*!
    \brief Parameters for GPU-driven mesh shader dispatch

    Mesh shader dispatch with parameters in a GPU buffer.

    \sa RenderPassCommandRecorder::drawMeshTasksIndirect()
*/
struct DrawMeshIndirectCommand {
    Handle<Buffer_t> buffer; ///< Buffer containing VkDrawMeshTasksIndirectCommandEXT structures
    size_t offset{ 0 }; ///< Byte offset into buffer
    uint32_t drawCount{ 0 }; ///< Number of mesh shader dispatches
    uint32_t stride{ 0 }; ///< Byte stride between consecutive commands
};

/*!
    \class RenderPassCommandRecorder
    \brief Records rendering commands within a render pass
    \ingroup public
    \headerfile render_pass_command_recorder.h <KDGpu/render_pass_command_recorder.h>


    RenderPassCommandRecorder is used to record GPU drawing commands that render to one or more
    attachments (textures).
    It cannot be created directly: instances are obtained by calling
    CommandRecorder::beginRenderPass() with appropriate options.

    ## Overview

    A render pass defines:
    - Color attachments (render targets)
    - Optional depth/stencil attachment
    - Load/store operations for each attachment
    - MSAA configuration and resolve targets
    - Multi-view configuration for VR/stereo rendering

    ## Creation

    \note RenderPassCommandRecorder cannot be instantiated directly. It must be created via
    CommandRecorder::beginRenderPass() using one of three option structs:
    - RenderPassCommandRecorderWithDynamicRenderingOptions (recommended - Vulkan 1.3 core)
    - RenderPassCommandRecorderOptions (legacy compatibility, creates implicit VkRenderPass internally)
    - RenderPassCommandRecorderWithRenderPassOptions (for explicit VkRenderPass objects, useful when using subpasses)
    .
    <br/>

    \warning The legacy VkRenderPass/VkFramebuffer model (via RenderPassCommandRecorderOptions or RenderPassCommandRecorderWithRenderPassOptions)
    has been marked as deprecated in Vulkan 1.3 in favor of dynamic rendering. On desktop platforms, new code should use
    RenderPassCommandRecorderWithDynamicRenderingOptions.

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

    ### Advanced Features

    <b>Multiple Render Targets (MRT):</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_multiple_attachments

    <b>Load Operations:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_load_operations

    <b>Stencil Operations:</b>

    \snippet kdgpu_doc_snippets.cpp renderpass_stencil_reference


    ## Lifecycle

    1. Create via CommandRecorder::beginRenderPass()
    2. Set pipeline state (pipeline, vertex buffers, bind groups, viewport, etc.)
    3. Issue draw commands
    4. Call end() to finish the render pass
    5. The RenderPassCommandRecorder is no longer valid after end()

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
    - RenderPassCommandRecorder::end() → vkCmdEndRendering() or vkCmdEndRenderPass()


    ## See also:
    \sa CommandRecorder, RenderPassCommandRecorderWithDynamicRenderingOptions, RenderPassCommandRecorderOptions, RenderPassCommandRecorderWithRenderPassOptions
    \sa ColorAttachment, DepthStencilAttachment, GraphicsPipeline
*/
class KDGPU_EXPORT RenderPassCommandRecorder
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
        \brief Binds a graphics pipeline for subsequent draw commands
        \param pipeline The graphics pipeline to bind

        All subsequent draw commands will use this pipeline's shaders, blend state,
        depth/stencil state, etc. Must be called before drawing.

        Vulkan: vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)
    */
    void setPipeline(const RequiredHandle<GraphicsPipeline_t> &pipeline);

    // TODO: Add overload for setting many vertex buffers at once
    /*!
        \brief Binds a vertex buffer to a binding index
        \param index Vertex buffer binding index (must match vertex input in pipeline)
        \param buffer The vertex buffer to bind
        \param offset Byte offset into the buffer (default 0)

        Subsequent draw commands will read vertex data from this buffer.

        Vulkan: vkCmdBindVertexBuffers()
    */
    void setVertexBuffer(uint32_t index, const RequiredHandle<Buffer_t> &buffer, DeviceSize offset = 0);

    /*!
        \brief Binds an index buffer for indexed draw commands
        \param buffer The index buffer to bind
        \param offset Byte offset into the buffer (default 0)
        \param indexType Index format: Uint16 or Uint32 (default Uint32)

        Required for drawIndexed() and drawIndexedIndirect() commands.

        Vulkan: vkCmdBindIndexBuffer()
    */
    void setIndexBuffer(const RequiredHandle<Buffer_t> &buffer, DeviceSize offset = 0, IndexType indexType = IndexType::Uint32);

    /*!
        \brief Binds a descriptor set (bind group) for shader resource access
        \param group Binding index (must match layout in pipeline)
        \param bindGroup The bind group containing resources (textures, buffers, samplers)
        \param pipelineLayout Optional pipeline layout (can be inferred from pipeline if omitted)
        \param dynamicBufferOffsets Offsets for dynamic uniform/storage buffers

        Shaders access resources through bind groups. Each bind group corresponds to a
        \c set in GLSL/HLSL shader code.

        Vulkan: vkCmdBindDescriptorSets()
    */
    void setBindGroup(uint32_t group,
                      const RequiredHandle<BindGroup_t> &bindGroup,
                      const OptionalHandle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>(),
                      std::span<const uint32_t> dynamicBufferOffsets = {});

    /*!
        \brief Sets the viewport transformation
        \param viewport Viewport rectangle and depth range

        Transforms normalized device coordinates to framebuffer coordinates.
        Can be set dynamically if the pipeline was created with dynamic viewport state.

        Vulkan: vkCmdSetViewport()
    */
    void setViewport(const Viewport &viewport);

    /*!
        \brief Sets the scissor rectangle for fragment clipping
        \param scissor Scissor rectangle in framebuffer coordinates

        Fragments outside the scissor rectangle are discarded. Useful for UI clipping
        or rendering to sub-regions.

        Vulkan: vkCmdSetScissor()
    */
    void setScissor(const Rect2D &scissor);

    /*!
        \brief Sets the stencil reference value for stencil testing
        \param faceMask Which faces to update (Front, Back, or FrontAndBack)
        \param reference The stencil reference value (0-255)

        Used with stencil operations for masking and stencil-based effects.

        Vulkan: vkCmdSetStencilReference()
    */
    void setStencilReference(StencilFaceFlags faceMask, int reference);

    /*!
        \brief Draws non-indexed geometry
        \param drawCommand Draw parameters (vertex count, instance count, etc.)

        Draws vertices directly from vertex buffers without an index buffer.

        Vulkan: vkCmdDraw()
    */
    void draw(const DrawCommand &drawCommand);

    /*!
        \brief Draws multiple non-indexed primitives in a single call
        \param drawCommands Array of draw commands to execute

        More efficient than issuing multiple draw() calls separately.

        Vulkan: Multiple vkCmdDraw() calls
    */
    void draw(std::span<const DrawCommand> drawCommands);

    /*!
        \brief Draws indexed geometry
        \param drawCommand Draw parameters (index count, instance count, etc.)

        Most commonly used draw command. Reads indices from the bound index buffer
        which reference vertices in vertex buffers.

        Vulkan: vkCmdDrawIndexed()
    */
    void drawIndexed(const DrawIndexedCommand &drawCommand);

    /*!
        \brief Draws multiple indexed primitives in a single call
        \param drawCommands Array of indexed draw commands

        Vulkan: Multiple vkCmdDrawIndexed() calls
    */
    void drawIndexed(std::span<const DrawIndexedCommand> drawCommands);

    /*!
        \brief Draws geometry with parameters in a GPU buffer (indirect rendering)
        \param drawCommand Indirect draw parameters

        Draw parameters are read from a GPU buffer, enabling GPU-driven rendering.

        Vulkan: vkCmdDrawIndirect()
    */
    void drawIndirect(const DrawIndirectCommand &drawCommand);
    void drawIndirect(std::span<const DrawIndirectCommand> drawCommands);

    /*!
        \brief Draws indexed geometry with parameters in a GPU buffer
        \param drawCommand Indirect indexed draw parameters

        Vulkan: vkCmdDrawIndexedIndirect()
    */
    void drawIndexedIndirect(const DrawIndexedIndirectCommand &drawCommand);
    void drawIndexedIndirect(std::span<const DrawIndexedIndirectCommand> drawCommands);

    /*!
        \brief Dispatches mesh shader work groups
        \param drawCommand Mesh shader dispatch parameters

        Requires mesh shading pipeline and VK_EXT_mesh_shader support.
        Mesh shaders replace the traditional vertex/geometry shader stages.

        Vulkan: vkCmdDrawMeshTasksEXT()
    */
    void drawMeshTasks(const DrawMeshCommand &drawCommand);
    void drawMeshTasks(std::span<const DrawMeshCommand> drawCommands);

    /*!
        \brief Dispatches mesh shader work groups with GPU-driven parameters
        \param drawCommand Indirect mesh shader dispatch

        Vulkan: vkCmdDrawMeshTasksIndirectEXT()
    */
    void drawMeshTasksIndirect(const DrawMeshIndirectCommand &drawCommand);
    void drawMeshTasksIndirect(std::span<const DrawMeshIndirectCommand> drawCommands);

    /*!
        \brief Uploads small data directly to shaders without buffers
        \param constantRange Push constant range (offset, size, shader stages)
        \param data Pointer to data to upload
        \param pipelineLayout Optional pipeline layout

        Push constants provide fast access to small amounts of frequently-updated data
        (typically up to 128 bytes). Much faster than updating uniform buffers.

        Vulkan: vkCmdPushConstants()
    */
    void pushConstant(const PushConstantRange &constantRange, const void *data, const OptionalHandle<PipelineLayout_t> &pipelineLayout = {});

    /*!
        \brief Dynamically creates and binds a temporary descriptor set
        \param group Binding index
        \param bindGroupEntries Resources to bind
        \param pipelineLayout Pipeline layout

        Convenience function for binding resources without pre-creating a BindGroup.

        Vulkan: vkCmdPushDescriptorSetKHR() (requires VK_KHR_push_descriptor)
    */
    void pushBindGroup(uint32_t group,
                       std::span<const BindGroupEntry> bindGroupEntries,
                       const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>());

    /*!
        \brief Advances to the next subpass (legacy render passes only)

        Only valid when using RenderPassCommandRecorderWithRenderPassOptions.
        Not applicable to dynamic rendering.

        \deprecated Subpasses are part of the legacy VkRenderPass model

        Vulkan: vkCmdNextSubpass()
    */
    void nextSubpass();

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

    /*!
        \brief Ends the render pass and finalizes recorded commands

        Must be called to finish the render pass. After calling end(), this
        RenderPassCommandRecorder is no longer valid and should not be used.

        Vulkan: vkCmdEndRendering() or vkCmdEndRenderPass()
    */
    void end();

private:
    explicit RenderPassCommandRecorder(GraphicsApi *api,
                                       const Handle<Device_t> &device,
                                       const Handle<RenderPassCommandRecorder_t> &renderPassCommandRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<RenderPassCommandRecorder_t> m_renderPassCommandRecorder;

    friend class CommandRecorder;
};

} // namespace KDGpu
