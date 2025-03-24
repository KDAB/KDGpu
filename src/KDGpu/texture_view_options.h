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
    std::string_view label;
    ViewType viewType{ ViewType::ViewType2D };
    Format format{ Format::UNDEFINED };
    TextureSubresourceRange range{};
    Handle<YCbCrConversion_t> yCbCrConversion{};
};

} // namespace KDGpu
