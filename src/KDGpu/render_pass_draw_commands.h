/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>

#include <cstdint>

namespace KDGpu {

struct Buffer_t;

/*!
    \brief Parameters for non-indexed draw commands

    Used with RenderPassCommandRecorder::draw() and DynamicRenderPassCommandRecorder::draw()
    to draw vertices directly from vertex buffers without using an index buffer.

    \sa RenderPassCommandRecorder::draw(), DynamicRenderPassCommandRecorder::draw()
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

    \sa RenderPassCommandRecorder::drawIndexed(), DynamicRenderPassCommandRecorder::drawIndexed(),
        RenderPassCommandRecorder::setIndexBuffer(), DynamicRenderPassCommandRecorder::setIndexBuffer()
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

    \sa RenderPassCommandRecorder::drawIndirect(), DynamicRenderPassCommandRecorder::drawIndirect()
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

    \sa RenderPassCommandRecorder::drawIndexedIndirect(), DynamicRenderPassCommandRecorder::drawIndexedIndirect()
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

    \sa RenderPassCommandRecorder::drawMeshTasks(), DynamicRenderPassCommandRecorder::drawMeshTasks()
*/
struct DrawMeshCommand {
    uint32_t workGroupX{ 1 }; ///< X dimension of mesh shader work groups
    uint32_t workGroupY{ 1 }; ///< Y dimension of mesh shader work groups
    uint32_t workGroupZ{ 1 }; ///< Z dimension of mesh shader work groups
};

/*!
    \brief Parameters for GPU-driven mesh shader dispatch

    Mesh shader dispatch with parameters in a GPU buffer.

    \sa RenderPassCommandRecorder::drawMeshTasksIndirect(), DynamicRenderPassCommandRecorder::drawMeshTasksIndirect()
*/
struct DrawMeshIndirectCommand {
    Handle<Buffer_t> buffer; ///< Buffer containing VkDrawMeshTasksIndirectCommandEXT structures
    size_t offset{ 0 }; ///< Byte offset into buffer
    uint32_t drawCount{ 0 }; ///< Number of mesh shader dispatches
    uint32_t stride{ 0 }; ///< Byte stride between consecutive commands
};

} // namespace KDGpu
