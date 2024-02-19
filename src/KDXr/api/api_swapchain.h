/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>

#include <KDGpu/handle.h>
#include <KDGpu/texture.h>

#include <vector>

namespace KDXr {

struct Swapchain_t;

/**
 * @brief ApiSwapchain
 * \ingroup api
 *
 */
struct ApiSwapchain {
    virtual std::vector<KDGpu::Texture> getTextures() = 0;
    virtual AcquireSwapchainTextureResult getNextTextureIndex(uint32_t &textureIndex) = 0;
    virtual WaitSwapchainTextureResult waitForTexture(Duration timeout = InfiniteDuration) const = 0;
    virtual ReleaseTextureResult releaseTexture() = 0;
};

} // namespace KDXr
