/*
This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#pragma once

#include <cstdint>

namespace KDGpu {
enum class ApiType : uint8_t {
    Vulkan = 0,
    UserDefined = 255
};
}
