/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>

namespace KDGpu {

struct BindGroup_t;
struct PushConstantRange;
struct PipelineLayout_t;
struct RayTracingPipeline_t;
struct RayTracingCommand;

/**
 * @brief ApiRayTracingPassCommandRecorder
 * \ingroup api
 *
 */
struct ApiRayTracingPassCommandRecorder {
    virtual void setPipeline(const Handle<RayTracingPipeline_t> &pipeline) = 0;
    virtual void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                              const Handle<PipelineLayout_t> &pipelineLayout,
                              const std::vector<uint32_t> &dynamicBufferOffsets) = 0;
    virtual void traceRays(const RayTracingCommand &rayTracingCommand) = 0;
    virtual void pushConstant(const PushConstantRange &constantRange, const void *data) = 0;
    virtual void end() = 0;
};

} // namespace KDGpu
