/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>
#include <KDXr/kdxr_export.h>

#include <KDGpu/gpu_core.h>
#include <KDGpu/texture.h>

#include <vector>

namespace KDXr {

struct Session_t;
struct Swapchain_t;
class XrApi;

/**
    @brief Holds option fields used for Swapchain creation
    @ingroup public
    @headerfile swapchain.h <KDXr/swapchain.h>
 */
struct SwapchainOptions {
    KDGpu::Format format{ KDGpu::Format::UNDEFINED };
    SwapchainUsageFlags usage{ SwapchainUsageFlagBits::ColorAttachmentBit | SwapchainUsageFlagBits::SampledBit };
    uint32_t width{ 0 };
    uint32_t height{ 0 };
    uint32_t arrayLayers{ 1 };
    uint32_t faceCount{ 1 };
    uint32_t mipLevels{ 1 };
    uint32_t sampleCount{ 1 };
};

class KDXR_EXPORT Swapchain
{
public:
    Swapchain();
    ~Swapchain();

    Swapchain(Swapchain &&);
    Swapchain &operator=(Swapchain &&);

    Swapchain(const Swapchain &) = delete;
    Swapchain &operator=(const Swapchain &) = delete;

    Handle<Swapchain_t> handle() const noexcept { return m_swapchain; }
    bool isValid() const { return m_swapchain.isValid(); }

    operator Handle<Swapchain_t>() const noexcept { return m_swapchain; }

    const std::vector<KDGpu::Texture> &textures() const { return m_textures; }

    AcquireSwapchainTextureResult getNextTextureIndex(uint32_t &textureIndex);
    WaitSwapchainTextureResult waitForTexture(Duration timeout = InfiniteDuration) const;

private:
    explicit Swapchain(XrApi *api, const Handle<Session_t> &sessionHandle, const SwapchainOptions &options);

    XrApi *m_api{ nullptr };
    Handle<Session_t> m_sessionHandle;
    Handle<Swapchain_t> m_swapchain;
    std::vector<KDGpu::Texture> m_textures;

    friend class Session;
};

} // namespace KDXr
