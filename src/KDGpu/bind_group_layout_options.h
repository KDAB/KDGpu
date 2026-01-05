/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/bind_group_description.h>
#include <KDFoundation/hashutils.h>

namespace KDGpu {

struct Sampler_t;
struct ResourceBindingLayout {
    uint32_t binding;
    uint32_t count{ 1 };
    ResourceBindingType resourceType;
    ShaderStageFlags shaderStages;
    ResourceBindingFlags flags{ ResourceBindingFlagBits::None };
    std::vector<Handle<Sampler_t>> immutableSamplers;

    friend bool operator==(const ResourceBindingLayout &, const ResourceBindingLayout &) = default;
};

// The following struct describes a bind group (descriptor set) layout and from this we
// will be able to subsequently allocate the actual bind group (descriptor set). Before
// the bind group can be used we will need to populate it with the specified bindings.
struct BindGroupLayoutOptions {
    std::string_view label;
    std::vector<ResourceBindingLayout> bindings;
    BindGroupLayoutFlags flags{ BindGroupLayoutFlagBits::None };

    // Equality operator for caching
    friend bool operator==(const BindGroupLayoutOptions &lhs, const BindGroupLayoutOptions &rhs) = default;
};

} // namespace KDGpu

// Hash function for BindGroupLayoutOptions to enable caching
namespace std {

template<>
struct hash<KDGpu::BindGroupLayoutOptions> {
    size_t operator()(const KDGpu::BindGroupLayoutOptions &options) const noexcept
    {
        uint64_t hash = 0;
        KDFoundation::hash_combine(hash, std::hash<std::string_view>()(options.label));
        for (const auto &binding : options.bindings) {
            KDFoundation::hash_combine(hash, binding.binding);
            KDFoundation::hash_combine(hash, binding.count);
            KDFoundation::hash_combine(hash, static_cast<uint32_t>(binding.resourceType));
            KDFoundation::hash_combine(hash, binding.shaderStages.toInt());
            KDFoundation::hash_combine(hash, binding.flags.toInt());
            for (const auto &sampler : binding.immutableSamplers) {
                KDFoundation::hash_combine(hash, sampler);
            }
        }
        return hash;
    }
};

} // namespace std
