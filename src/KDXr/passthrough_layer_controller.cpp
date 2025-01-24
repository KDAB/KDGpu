/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "passthrough_layer_controller.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_passthrough_layer.h>
#include <KDXr/api/api_session.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class PassthroughLayer
    @brief PassthroughLayer is used to initialize the XR API.
    @ingroup public
    @headerfile reference_space.h <KDXr/reference_space.h>

    @sa PassthroughLayerOptions
 */

/**
    @fn PassthroughLayer::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific PassthroughLayer

    @return KDGpu::Handle<PassthroughLayer_t>
    @sa ResourceManager
 */

/**
    @fn PassthroughLayer::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

PassthroughLayerController::PassthroughLayerController()
{
}

PassthroughLayerController::PassthroughLayerController(const KDGpu::Handle<Session_t> &sessionHandle, XrApi *api, const PassthroughLayerOptions &options)
    : m_api(api)
    , m_sessionHandle(sessionHandle)
{
    // Create an PassthroughLayer using the underlying API
    m_passthroughLayer = m_api->resourceManager()->createPassthroughLayer(m_sessionHandle, options);
}

PassthroughLayerController::~PassthroughLayerController()
{
    if (isValid())
        m_api->resourceManager()->deletePassthroughLayer(handle());
}

PassthroughLayerController::PassthroughLayerController(PassthroughLayerController &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_sessionHandle = std::exchange(other.m_sessionHandle, {});
    m_passthroughLayer = std::exchange(other.m_passthroughLayer, {});
}

PassthroughLayerController &PassthroughLayerController::operator=(PassthroughLayerController &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deletePassthroughLayer(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_sessionHandle = std::exchange(other.m_sessionHandle, {});
        m_passthroughLayer = std::exchange(other.m_passthroughLayer, {});
    }
    return *this;
}

void PassthroughLayerController::setRunning(bool running)
{
    auto passthroughLayer = m_api->resourceManager()->getPassthroughLayer(m_passthroughLayer);

    passthroughLayer->setRunning(running);
}

} // namespace KDXr
