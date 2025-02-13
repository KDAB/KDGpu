/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>
#include <KDXr/swapchain.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

namespace KDXr {

class OpenXrResourceManager;
struct Swapchain_t;

/**
 * @brief OpenXrSwapchain
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSwapchain {
    explicit OpenXrSwapchain(OpenXrResourceManager *_openxrResourceManager,
                             XrSwapchain _swapchain,
                             const KDGpu::Handle<Session_t> &_sessionHandle,
                             const SwapchainOptions &_options) noexcept;

    std::vector<KDGpu::Texture> getTextures();
    AcquireSwapchainTextureResult getNextTextureIndex(uint32_t &textureIndex);
    WaitSwapchainTextureResult waitForTexture(Duration timeout = InfiniteDuration) const;
    ReleaseTextureResult releaseTexture();

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSwapchain swapchain{ XR_NULL_HANDLE };
    KDGpu::Handle<Session_t> sessionHandle;
    SwapchainOptions options;
};

} // namespace KDXr
