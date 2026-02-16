/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2026 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <span>
#include <string_view>
#include <cstdint>

namespace KDGpu {

struct PipelineCacheOptions {
    std::string_view label;
    std::span<const uint8_t> initialData; // Initial cache data to populate from previous runs
};

} // namespace KDGpu
