/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/graphics_pipeline_options.h>

#include <optional>

namespace KDGpu {

struct PipelineLayout_t;
struct ShaderModule_t;

struct RayTracingShaderGroupOptions {
    RayTracingShaderGroupType type;
    // clang-format off
    // indices into the RayTracingPipelineOptions::shaderStages vector
    std::optional<uint32_t> generalShaderIndex;      // Only set if type == General
    std::optional<uint32_t> closestHitShaderIndex;   // Optional set if type == Procedural || Triangle
    std::optional<uint32_t> anyHitShaderIndex;       // Optional set if type == Procedural || Triangle
    std::optional<uint32_t> intersectionShaderIndex; // Optional set if type == Procedural
    // clang-format on
};

struct RayTracingPipelineOptions {
    std::string_view label;
    std::vector<ShaderStage> shaderStages;
    std::vector<RayTracingShaderGroupOptions> shaderGroups;
    Handle<PipelineLayout_t> layout;
    uint32_t maxRecursionDepth{ 0 }; // 0 will default to maxRecursionDepthLimit
};

} // namespace KDGpu
