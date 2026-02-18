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

namespace KDGpu {

struct Device_t;
struct PipelineLayout_t;
struct PipelineLayoutOptions;

/*!
    \class PipelineLayout
    \brief Defines the interface between pipeline and shader resources
    \ingroup public
    \headerfile pipeline_layout.h <KDGpu/pipeline_layout.h>

    <b>Vulkan equivalent:</b> [VkPipelineLayout](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineLayout.html)

    PipelineLayout describes the complete shader interface: which descriptor sets can be bound and
    what push constant ranges are available. It must be compatible with both the pipeline and the
    bind groups used.

    <b>Key features:</b>
    - Specify bind group layouts (descriptor sets)
    - Define push constant ranges
    - Must match shader declarations
    - Reusable across compatible pipelines
    .
    <br>

    <b>Lifetime:</b> PipelineLayouts are created by Device and must remain valid while referenced
    by pipelines. They use RAII and clean up automatically.

    ## Usage

    <b>Creating a basic pipeline layout:</b>

    \snippet kdgpu_doc_snippets.cpp pipelinelayout_simple

    <b>Pipeline layout with push constants:</b>

    \snippet kdgpu_doc_snippets.cpp pipelinelayout_push_constants

    <b>Multiple bind group layouts:</b>

    \snippet kdgpu_doc_snippets.cpp pipelinelayout_multiple_sets

    <b>PipelineLayout compatibility:</b>

    \snippet kdgpu_doc_snippets.cpp pipelinelayout_compatibility

    <b>Sharing layouts between pipelines:</b>

    \snippet kdgpu_doc_snippets.cpp pipelinelayout_partial_update

    ## Vulkan mapping:
    - PipelineLayout creation -> vkCreatePipelineLayout()
    - Used in VkGraphicsPipelineCreateInfo and VkComputePipelineCreateInfo
    - Used in vkCmdBindDescriptorSets()

    ## See also:
    \sa PipelineLayoutOptions, BindGroupLayout, GraphicsPipeline, ComputePipeline, BindGroup, Device
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT PipelineLayout
{
public:
    PipelineLayout();
    ~PipelineLayout();

    PipelineLayout(PipelineLayout &&) noexcept;
    PipelineLayout &operator=(PipelineLayout &&) noexcept;

    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout &operator=(const PipelineLayout &) = delete;

    const Handle<PipelineLayout_t> &handle() const noexcept { return m_pipelineLayout; }
    bool isValid() const noexcept { return m_pipelineLayout.isValid(); }

    operator Handle<PipelineLayout_t>() const noexcept { return m_pipelineLayout; }

private:
    explicit PipelineLayout(GraphicsApi *api,
                            const Handle<Device_t> &device,
                            const PipelineLayoutOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<PipelineLayout_t> m_pipelineLayout;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const PipelineLayout &, const PipelineLayout &);
};

KDGPU_EXPORT bool operator==(const PipelineLayout &a, const PipelineLayout &b);
KDGPU_EXPORT bool operator!=(const PipelineLayout &a, const PipelineLayout &b);

} // namespace KDGpu
