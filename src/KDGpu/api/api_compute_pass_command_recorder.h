/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>

namespace KDGpu {

struct BindGroup_t;
struct ComputePipeline_t;
struct ComputeCommand;
struct ComputeCommandIndirect;
struct PushConstantRange;
struct PipelineLayout_t;

/**
 * @brief ApiComputePassCommandRecorder
 * \ingroup api
 *
 */
struct ApiComputePassCommandRecorder {
    virtual void setPipeline(const Handle<ComputePipeline_t> &pipeline) = 0;
    virtual void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                              const Handle<PipelineLayout_t> &pipelineLayout, const std::vector<uint32_t> &dynamicBufferOffsets) = 0;
    virtual void dispatchCompute(const ComputeCommand &command) = 0;
    virtual void dispatchCompute(const std::vector<ComputeCommand> &commands) = 0;
    virtual void dispatchComputeIndirect(const ComputeCommandIndirect &command) = 0;
    virtual void dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands) = 0;
    virtual void pushConstant(const PushConstantRange &constantRange, const void *data) = 0;
    virtual void end() = 0;
};

} // namespace KDGpu
