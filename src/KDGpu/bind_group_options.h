/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/bind_group_description.h>
#include <KDGpu/bind_group_layout.h>

namespace KDGpu {

struct BindGroupEntry { // An entry into a BindGroup ( == a descriptor in a descriptor set)
    uint32_t binding;
    BindingResource resource;
};

struct BindGroupOptions {
    std::string_view label;
    Handle<BindGroupLayout_t> layout;
    std::vector<BindGroupEntry> resources;
    uint32_t maxVariableArrayLength{ 0 };
};

} // namespace KDGpu
