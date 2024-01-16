/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "swapchain.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_swapchain.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class Swapchain
    @brief Swapchain is used to initialize the XR API.
    @ingroup public
    @headerfile swapchain.h <KDXr/swapchain.h>

    @sa SwapchainOptions
 */

/**
    @fn Swapchain::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific Swapchain

    @return Handle<Swapchain_t>
    @sa ResourceManager
 */

/**
    @fn Swapchain::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

Swapchain::Swapchain()
{
}

Swapchain::Swapchain(XrApi *api, const Handle<Session_t> &sessionHandle, const SwapchainOptions &options)
    : m_api(api)
    , m_sessionHandle(sessionHandle)
    , m_swapchain(m_api->resourceManager()->createSwapchain(m_sessionHandle, options))
{
    // Fetch the textures owned by the swapchain
    auto apiSwapchain = m_api->resourceManager()->getSwapchain(m_swapchain);
    m_textures = std::move(apiSwapchain->getTextures());
}

Swapchain::~Swapchain()
{
    if (isValid())
        m_api->resourceManager()->deleteSwapchain(handle());
}

Swapchain::Swapchain(Swapchain &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_sessionHandle = std::exchange(other.m_sessionHandle, {});
    m_swapchain = std::exchange(other.m_swapchain, {});
    m_textures = std::exchange(other.m_textures, {});
}

Swapchain &Swapchain::operator=(Swapchain &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSwapchain(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_sessionHandle = std::exchange(other.m_sessionHandle, {});
        m_swapchain = std::exchange(other.m_swapchain, {});
        m_textures = std::exchange(other.m_textures, {});
    }
    return *this;
}

} // namespace KDXr
