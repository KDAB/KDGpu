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

struct BufferOptions {
    DeviceSize size;
    BufferUsageFlags usage;
    MemoryUsage memoryUsage;
    SharingMode sharingMode{ SharingMode::Exclusive };
    std::vector<uint32_t> queueTypeIndices{};
    ExternalMemoryHandleTypeFlags externalMemoryHandleType{ ExternalMemoryHandleTypeFlagBits::None };
};

} // namespace KDGpu
