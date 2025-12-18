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

namespace KDGpu {

struct BindGroup_t;
struct Buffer_t;
struct Device_t;
struct ComputePipeline_t;
struct ComputePassCommandRecorder_t;
struct PipelineLayout_t;
struct BindGroupEntry;

struct PushConstantRange;

struct ComputeCommand {
    uint32_t workGroupX{ 1 };
    uint32_t workGroupY{ 1 };
    uint32_t workGroupZ{ 1 };
};

struct ComputeCommandIndirect {
    Handle<Buffer_t> buffer;
    size_t offset{ 0 };
};

struct ComputePassCommandRecorderOptions {
};

/**
 * @brief ComputePassCommandRecorder
 * @ingroup public
 */
class KDGPU_EXPORT ComputePassCommandRecorder
{
public:
    ~ComputePassCommandRecorder();

    ComputePassCommandRecorder(ComputePassCommandRecorder &&) noexcept;
    ComputePassCommandRecorder &operator=(ComputePassCommandRecorder &&) noexcept;

    ComputePassCommandRecorder(const ComputePassCommandRecorder &) = delete;
    ComputePassCommandRecorder &operator=(const ComputePassCommandRecorder &) = delete;

    const Handle<ComputePassCommandRecorder_t> &handle() const noexcept { return m_computePassCommandRecorder; }
    bool isValid() const noexcept { return m_computePassCommandRecorder.isValid(); }

    operator Handle<ComputePassCommandRecorder_t>() const noexcept { return m_computePassCommandRecorder; }

    void setPipeline(const Handle<ComputePipeline_t> &pipeline);

    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>(),
                      std::span<const uint32_t> dynamicBufferOffsets = {});

    void dispatchCompute(const ComputeCommand &command);
    void dispatchCompute(std::span<const ComputeCommand> commands);

    void dispatchComputeIndirect(const ComputeCommandIndirect &command);
    void dispatchComputeIndirect(std::span<const ComputeCommandIndirect> commands);

    void pushConstant(const PushConstantRange &constantRange, const void *data);
    void pushBindGroup(uint32_t group,
                       std::span<const BindGroupEntry> bindGroupEntries,
                       const Handle<PipelineLayout_t> &pipelineLayout = Handle<PipelineLayout_t>());

    void end();

private:
    explicit ComputePassCommandRecorder(GraphicsApi *api,
                                        const Handle<Device_t> &device,
                                        const Handle<ComputePassCommandRecorder_t> &computePassCommandRecorder);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<ComputePassCommandRecorder_t> m_computePassCommandRecorder;

    friend class CommandRecorder;
};

} // namespace KDGpu
