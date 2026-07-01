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
    /// \note The caller is responsible for ensuring the string referenced by \c label outlives this options object.
    std::string_view label;
    uint16_t uniformBufferCount{ 0 };
    uint16_t dynamicUniformBufferCount{ 0 };
    uint16_t storageBufferCount{ 0 };
    uint16_t textureSamplerCount{ 0 };
    uint16_t textureCount{ 0 };
    uint16_t samplerCount{ 0 };
    uint16_t imageCount{ 0 };
    uint16_t inputAttachmentCount{ 0 };
    uint16_t accelerationStructureCount{ 0 };

    uint16_t maxBindGroupCount{ 1 };

    BindGroupPoolFlags flags{ BindGroupPoolFlagBits::CreateFreeBindGroups };
};

} // namespace KDGpu
