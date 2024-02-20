/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/bind_group_description.h>

namespace KDGpu {

struct ResourceBindingLayout {
    uint32_t binding;
    uint32_t count{ 1 };
    ResourceBindingType resourceType;
    ShaderStageFlags shaderStages;
    ResourceBindingFlags flags{ ResourceBindingFlagBits::None };

    friend bool operator==(const ResourceBindingLayout &, const ResourceBindingLayout &) = default;
};

// The following struct describes a bind group (descriptor set) layout and from this we
// will be able to subsequently allocate the actual bind group (descriptor set). Before
// the bind group can be used we will need to populate it with the specified bindings.
struct BindGroupLayoutOptions {
    std::string_view label;
    std::vector<ResourceBindingLayout> bindings;
};

} // namespace KDGpu
