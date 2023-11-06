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
 * @brief ApiBuffer
 * \ingroup api
 *
 */
struct ApiBuffer {
    virtual void *map() = 0;
    virtual void unmap() = 0;
    virtual void invalidate() = 0;
    virtual void flush() = 0;
    virtual MemoryHandle externalMemoryHandle() const = 0;
};

} // namespace KDGpu
