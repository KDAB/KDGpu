/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct TextureOptions {
    TextureType type;
    Format format;
    Extent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers{ 1 };
    SampleCountFlagBits samples{ SampleCountFlagBits::Samples1Bit };
    TextureTiling tiling{ TextureTiling::Optimal };
    TextureUsageFlags usage;
    MemoryUsage memoryUsage;
    SharingMode sharingMode{ SharingMode::Exclusive };
    std::vector<uint32_t> queueTypeIndices{};
    TextureLayout initialLayout{ TextureLayout::Undefined };
    // TODO: TextureFlags flags;
};

} // namespace KDGpu
