/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct ApiFence {
    virtual void wait() = 0;
    virtual void reset() = 0;
    virtual FenceStatus status() = 0;
};

} // namespace KDGpu
