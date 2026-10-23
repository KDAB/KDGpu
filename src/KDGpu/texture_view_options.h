/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/ycbcr_conversion.h>

namespace KDGpu {

struct YCbCrConversion_t;

struct TextureViewOptions {
    /// \note The caller is responsible for ensuring the string referenced by \c label outlives this options object.
    std::string_view label;
    ViewType viewType{ ViewType::ViewType2D };
    Format format{ Format::UNDEFINED };
    TextureSubresourceRange range{};
    OptionalHandle<YCbCrConversion_t> yCbCrConversion;
    ComponentMapping components{
        .r = ComponentSwizzle::Identity,
        .g = ComponentSwizzle::Identity,
        .b = ComponentSwizzle::Identity,
        .a = ComponentSwizzle::Identity,
    };
};

} // namespace KDGpu
