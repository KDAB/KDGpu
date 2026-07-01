/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct PipelineLayout_t;
struct ShaderModule_t;
struct PipelineCache_t;

struct ComputeShaderStage {
    RequiredHandle<ShaderModule_t> shaderModule;
    std::string entryPoint{ "main" };
    std::vector<SpecializationConstant> specializationConstants;
};

struct ComputePipelineOptions {
    /// \note The caller is responsible for ensuring the string referenced by \c label outlives this options object.
    std::string_view label;
    RequiredHandle<PipelineLayout_t> layout;
    ComputeShaderStage shaderStage;

    // Optional pipeline cache to speed up pipeline creation
    OptionalHandle<PipelineCache_t> pipelineCache;
};

} // namespace KDGpu
