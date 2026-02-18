/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>

#include <KDGpu/kdgpu_export.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct Device_t;
struct GraphicsPipeline_t;
struct GraphicsPipelineOptions;

/*!
    \class GraphicsPipeline
    \brief Represents a complete graphics rendering pipeline state
    \ingroup public
    \headerfile graphics_pipeline.h <KDGpu/graphics_pipeline.h>

    <b>Vulkan equivalent:</b> [VkPipeline](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipeline.html) (graphics)

    GraphicsPipeline encapsulates all state needed for a draw call: shaders, vertex input layout,
    rasterization state, depth/stencil testing, blending, and more. In KDGpu, pipelines are immutable
    once created.

    <b>Key components:</b>
    - Shader stages (vertex, fragment, geometry, etc.)
    - Vertex input layout and attributes
    - Primitive topology (triangles, lines, points)
    - Viewport and scissor configuration
    - Rasterization state (culling, polygon mode)
    - Depth and stencil testing
    - Color blending and write masks
    - Pipeline layout (descriptor sets, push constants)
    .
    <br/>

    <b>Lifetime:</b> Pipelines are created by Device and must remain valid while referenced by render
    passes. They use RAII and clean up automatically.


    ## Usage

    <b>Creating a simple pipeline:</b>

    \snippet kdgpu_doc_snippets.cpp vertex_struct

    \snippet kdgpu_doc_snippets.cpp graphicspipeline_simple

    <b>Pipeline with depth testing:</b>

    \snippet kdgpu_doc_snippets.cpp graphicspipeline_depth

    <b>Alpha blending:</b>

    \snippet kdgpu_doc_snippets.cpp graphicspipeline_blending

    <b>Culling configuration:</b>

    \snippet kdgpu_doc_snippets.cpp graphicspipeline_culling

    <b>Multisampling:</b>

    \snippet kdgpu_doc_snippets.cpp graphicspipeline_multisampling

    ## Vulkan mapping:
    - GraphicsPipeline creation -> vkCreateGraphicsPipelines()
    - Bound with vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)

    ## See also:
    \sa Device, GraphicsPipelineOptions, PipelineLayout, ShaderModule, ComputePipeline
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT GraphicsPipeline
{
public:
    GraphicsPipeline();
    ~GraphicsPipeline();

    GraphicsPipeline(GraphicsPipeline &&) noexcept;
    GraphicsPipeline &operator=(GraphicsPipeline &&) noexcept;

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    const Handle<GraphicsPipeline_t> &handle() const noexcept { return m_graphicsPipeline; }
    bool isValid() const noexcept { return m_graphicsPipeline.isValid(); }

    operator Handle<GraphicsPipeline_t>() const noexcept { return m_graphicsPipeline; }

private:
    explicit GraphicsPipeline(GraphicsApi *api, const Handle<Device_t> &device, const GraphicsPipelineOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<GraphicsPipeline_t> m_graphicsPipeline;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const GraphicsPipeline &, const GraphicsPipeline &);
};

KDGPU_EXPORT bool operator==(const GraphicsPipeline &a, const GraphicsPipeline &b);
KDGPU_EXPORT bool operator!=(const GraphicsPipeline &a, const GraphicsPipeline &b);

} // namespace KDGpu
