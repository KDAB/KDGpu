/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct BindGroupPoolOptions {
    std::string_view label;
    uint16_t uniformBufferCount{ 1 };
    uint16_t dynamicUniformBufferCount{ 1 };
    uint16_t storageBufferCount{ 1 };
    uint16_t textureSamplerCount{ 1 };
    uint16_t textureCount{ 1 };
    uint16_t samplerCount{ 1 };
    uint16_t imageCount{ 1 };
    uint16_t inputAttachmentCount{ 1 };

    uint16_t maxBindGroupCount{ 1 };

    BindGroupPoolFlags flags{ BindGroupPoolFlagBits::CreateFreeBindGroups };
};

} // namespace KDGpu
