/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "swapchain.h"

#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_swapchain.h>
#include <KDGpu/swapchain_options.h>

namespace KDGpu {

Swapchain::Swapchain()
{
}

Swapchain::Swapchain(GraphicsApi *api, const Handle<Device_t> &device, const SwapchainOptions &options)
    : m_api(api)
    , m_device(device)
    , m_swapchain(m_api->resourceManager()->createSwapchain(m_device, options))
{
    // Fetch the textures owned by the swapchain
    auto apiSwapchain = m_api->resourceManager()->getSwapchain(m_swapchain);
    const auto textureHandles = apiSwapchain->getTextures();
    const uint32_t textureCount = textureHandles.size();
    m_textures.reserve(textureCount);
    for (uint32_t i = 0; i < textureCount; ++i)
        m_textures.emplace_back(Texture(m_api, m_device, textureHandles[i]));
}

Swapchain::Swapchain(Swapchain &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_swapchain = std::exchange(other.m_swapchain, {});
    m_textures = std::exchange(other.m_textures, {});
}

Swapchain &Swapchain::operator=(Swapchain &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSwapchain(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_swapchain = std::exchange(other.m_swapchain, {});
        m_textures = std::exchange(other.m_textures, {});
    }
    return *this;
}

Swapchain::~Swapchain()
{
    if (isValid())
        m_api->resourceManager()->deleteSwapchain(handle());
}

AcquireImageResult Swapchain::getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore)
{
    auto apiSwapchain = m_api->resourceManager()->getSwapchain(m_swapchain);
    return apiSwapchain->getNextImageIndex(imageIndex, semaphore);
}

} // namespace KDGpu
