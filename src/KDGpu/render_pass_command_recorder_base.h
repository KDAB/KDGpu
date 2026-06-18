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
#include <KDGpu/render_pass_draw_commands.h>

#include <span>
#include <optional>

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct GraphicsPipeline_t;
struct PipelineLayout_t;

struct Rect2D;
struct Viewport;
struct PushConstantRange;
struct BindGroupEntry;

struct VulkanRenderPassCommandRecorderBase;

/**
 * @brief Base class for render pass command recorders (NVI pattern).
 * \ingroup public
 *
 * All drawing commands are non-virtual and implemented here; they delegate to a
 * single protected pure-virtual backend() method overridden by each concrete class.
 *
 * \sa RenderPassCommandRecorder (legacy VkRenderPass path)
 * \sa DynamicRenderPassCommandRecorder (recommended modern path)
 */
class KDGPU_EXPORT RenderPassCommandRecorderBase
{
public:
    virtual ~RenderPassCommandRecorderBase() = default;

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
    void pushConstant(const PushConstantRange &constantRange, const void *data,
                      const OptionalHandle<PipelineLayout_t> &pipelineLayout = {});

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
        \brief Ends the render pass and finalizes recorded commands

        Must be called to finish the render pass. After calling end(), this
        RenderPassCommandRecorder is no longer valid and should not be used.

        Vulkan: vkCmdEndRendering() or vkCmdEndRenderPass()
    */
    void end();

protected:
    RenderPassCommandRecorderBase() = default;
    explicit RenderPassCommandRecorderBase(GraphicsApi *api, const Handle<Device_t> &device)
        : m_api(api), m_device(device)
    {
    }

    virtual VulkanRenderPassCommandRecorderBase *backend() const = 0;

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
};

} // namespace KDGpu
