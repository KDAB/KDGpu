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

struct PipelineLayout_t;
struct ShaderModule_t;

struct ComputeShaderStage {
    Handle<ShaderModule_t> shaderModule;
    std::string entryPoint{ "main" };
    std::vector<SpecializationConstant> specializationConstants;
};

struct ComputePipelineOptions {
    std::string_view label;
    Handle<PipelineLayout_t> layout;
    ComputeShaderStage shaderStage;
};

} // namespace KDGpu
