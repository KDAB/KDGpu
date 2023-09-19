/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

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
struct ApiTimestampQueryRecorder {
    virtual TimestampIndex writeTimestamp(PipelineStageFlags flags) = 0;
    virtual std::vector<uint64_t> queryResults() = 0;
    virtual void reset() = 0;
    virtual float timestampPeriod() const = 0;
};

} // namespace KDGpu
