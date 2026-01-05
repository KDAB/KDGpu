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

namespace std {

template<>
struct hash<KDGpu::PushConstantRange> {
    size_t operator()(const KDGpu::PushConstantRange &p) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, p.size);
        KDFoundation::hash_combine(hash, p.offset);
        KDFoundation::hash_combine(hash, p.shaderStages);
        return hash;
    }
};

template<>
struct hash<KDGpu::PipelineLayoutOptions> {
    size_t operator()(const KDGpu::PipelineLayoutOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, std::hash<std::string_view>()(options.label));
        for (const auto &layout : options.bindGroupLayouts) {
            KDFoundation::hash_combine(hash, layout);
        }
        for (const auto &pushConstantRange : options.pushConstantRanges) {
            KDFoundation::hash_combine(hash, pushConstantRange);
        }
        return hash;
    }
};

} // namespace std
