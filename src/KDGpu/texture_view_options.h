/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct Texture_t;

struct TextureViewOptions {
    ViewType viewType{ ViewType::ViewType2D };
    Format format{ Format::UNDEFINED };
    TextureSubresourceRange range{};
};

} // namespace KDGpu
