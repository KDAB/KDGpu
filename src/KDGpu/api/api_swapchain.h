/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/gpu_core.h>

#include <vector>

namespace KDGpu {

struct GpuSemaphore_t;
struct Texture_t;
/**
 * @brief ApiSwapchain
 *
 */
struct ApiSwapchain {
    virtual std::vector<Handle<Texture_t>> getTextures() = 0;
    virtual AcquireImageResult getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore) = 0;
};

} // namespace KDGpu
