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
struct ComputePipeline_t;
struct ComputePipelineOptions;

/*!
    \class ComputePipeline
    \brief Represents a compute shader pipeline for general GPU computation
    \ingroup public
    \headerfile compute_pipeline.h <KDGpu/compute_pipeline.h>

    <b>Vulkan equivalent:</b> [VkPipeline](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipeline.html) (compute)

    ComputePipeline encapsulates a compute shader and its associated resources. Compute shaders run
    on the GPU outside of the graphics pipeline, enabling general-purpose GPU computation (GPGPU).

    <b>Key features:</b>
    - Parallel computation on GPU
    - Access to buffers and images for read/write
    - Work group based execution model
    - Shared memory within work groups
    .
    <br/>

    <b>Lifetime:</b> Compute pipelines are created by Device and must remain valid while referenced
    by compute passes. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a compute pipeline:</b>

    \snippet kdgpu_doc_snippets.cpp computepipeline_creation

    <b>Dispatching compute work:</b>

    \snippet kdgpu_doc_snippets.cpp computepipeline_dispatch

    <b>Particle system example:</b>

    \snippet kdgpu_doc_snippets.cpp computepipeline_particle_system

    <b>Specialization constants:</b>

    \snippet kdgpu_doc_snippets.cpp computepipeline_specialization

    <b>Image processing:</b>

    \snippet kdgpu_doc_snippets.cpp computepipeline_image_processing

    ## Vulkan mapping:
    - ComputePipeline creation -> vkCreateComputePipelines()
    - Bound with vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE)
    - Dispatched with vkCmdDispatch()

    ## See also:
    \sa Device, ComputePipelineOptions, PipelineLayout, ShaderModule, ComputePassCommandRecorder, GraphicsPipeline
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT ComputePipeline
{
public:
    ComputePipeline();
    ~ComputePipeline();

    ComputePipeline(ComputePipeline &&) noexcept;
    ComputePipeline &operator=(ComputePipeline &&) noexcept;

    ComputePipeline(const ComputePipeline &) = delete;
    ComputePipeline &operator=(const ComputePipeline &) = delete;

    const Handle<ComputePipeline_t> &handle() const noexcept { return m_computePipeline; }
    bool isValid() const noexcept { return m_computePipeline.isValid(); }

    operator Handle<ComputePipeline_t>() const noexcept { return m_computePipeline; }

private:
    explicit ComputePipeline(GraphicsApi *api,
                             const Handle<Device_t> &device,
                             const ComputePipelineOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ComputePipeline_t> m_computePipeline;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const ComputePipeline &, const ComputePipeline &);
};

KDGPU_EXPORT bool operator==(const ComputePipeline &a, const ComputePipeline &b);
KDGPU_EXPORT bool operator!=(const ComputePipeline &a, const ComputePipeline &b);

} // namespace KDGpu
