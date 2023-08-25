/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/bind_group_layout.h>
#include <KDGpu/handle.h>

#include <vector>

namespace KDGpu {

struct PushConstantRange {
    uint32_t offset;
    uint32_t size;
    ShaderStageFlags shaderStages;

    friend bool operator==(const PushConstantRange &, const PushConstantRange &) = default;
};

struct BindGroupLayout_t;

struct PipelineLayoutOptions {
    std::string_view label;
    std::vector<Handle<BindGroupLayout_t>> bindGroupLayouts;
    std::vector<PushConstantRange> pushConstantRanges;
};

} // namespace KDGpu
