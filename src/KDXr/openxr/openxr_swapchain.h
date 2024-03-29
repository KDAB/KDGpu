/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/api/api_swapchain.h>
#include <KDXr/kdxr_export.h>
#include <KDXr/config.h>
#include <KDXr/swapchain.h>

#include <KDGpu/handle.h>

#include <openxr/openxr.h>

namespace KDXr {

class OpenXrResourceManager;

/**
 * @brief OpenXrSwapchain
 * \ingroup openxr
 *
 */
struct KDXR_EXPORT OpenXrSwapchain : public ApiSwapchain {
    explicit OpenXrSwapchain(OpenXrResourceManager *_openxrResourceManager,
                             XrSwapchain _swapchain,
                             const KDGpu::Handle<Session_t> &_sessionHandle,
                             const SwapchainOptions &_options) noexcept;

    std::vector<KDGpu::Texture> getTextures() final;
    AcquireSwapchainTextureResult getNextTextureIndex(uint32_t &textureIndex) final;
    WaitSwapchainTextureResult waitForTexture(Duration timeout = InfiniteDuration) const final;
    ReleaseTextureResult releaseTexture() final;

    OpenXrResourceManager *openxrResourceManager{ nullptr };
    XrSwapchain swapchain{ XR_NULL_HANDLE };
    KDGpu::Handle<Session_t> sessionHandle;
    SwapchainOptions options;
};

} // namespace KDXr
