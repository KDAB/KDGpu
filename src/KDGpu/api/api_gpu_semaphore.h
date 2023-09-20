/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

/**
 * @brief ApiGpuSemaphore
 * \ingroup api
 *
 */
struct ApiGpuSemaphore {

    virtual HandleOrFD externalSemaphoreHandle() const = 0;
};

} // namespace KDGpu
