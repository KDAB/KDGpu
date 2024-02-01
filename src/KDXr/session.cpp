/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "session.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_session.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class Session
    @brief Session is used to initialize the XR API.
    @ingroup public
    @headerfile session.h <KDXr/session.h>

    @sa SessionOptions
 */

/**
    @fn Session::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific Session

    @return Handle<Session_t>
    @sa ResourceManager
 */

/**
    @fn Session::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

Session::Session()
{
}

Session::Session(const Handle<System_t> &systemHandle, XrApi *api, const SessionOptions &options)
    : m_api(api)
    , m_systemHandle(systemHandle)
{
    // Create an Session using the underlying API
    m_session = m_api->resourceManager()->createSession(m_systemHandle, options);
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    apiSession->initialize(this);
}

Session::~Session()
{
    if (isValid())
        m_api->resourceManager()->deleteSession(handle());
}

Session::Session(Session &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_systemHandle = std::exchange(other.m_systemHandle, {});
    m_session = std::exchange(other.m_session, {});

    auto apiSession = m_api->resourceManager()->getSession(m_session);
    apiSession->initialize(this);
}

Session &Session::operator=(Session &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSession(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_systemHandle = std::exchange(other.m_systemHandle, {});
        m_session = std::exchange(other.m_session, {});

        auto apiSession = m_api->resourceManager()->getSession(m_session);
        apiSession->initialize(this);
    }
    return *this;
}

ReferenceSpace Session::createReferenceSpace(const ReferenceSpaceOptions &options)
{
    return ReferenceSpace(m_session, m_api, options);
}

std::span<const KDGpu::Format> Session::supportedSwapchainFormats() const
{
    if (m_supportedSwapchainFormats.empty()) {
        auto apiSession = m_api->resourceManager()->getSession(m_session);
        m_supportedSwapchainFormats = apiSession->supportedSwapchainFormats();
    }
    return m_supportedSwapchainFormats;
}

KDGpu::Format Session::selectSwapchainFormat(std::span<const KDGpu::Format> preferredFormats) const
{
    auto availableFormats = supportedSwapchainFormats();
    for (const auto &swapchainFormat : preferredFormats) {
        if (std::find(availableFormats.begin(), availableFormats.end(), swapchainFormat) != availableFormats.end())
            return swapchainFormat;
    }

    SPDLOG_LOGGER_ERROR(Logger::logger(), "No supported swapchain format found");
    return KDGpu::Format::UNDEFINED;
}

Swapchain Session::createSwapchain(const SwapchainOptions &options)
{
    return Swapchain(m_api, m_session, options);
}

FrameState Session::waitForFrame()
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->waitForFrame();
}

BeginFrameResult Session::beginFrame()
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->beginFrame();
}

EndFrameResult Session::endFrame(const EndFrameOptions &options)
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->endFrame(options);
}

LocateViewsResult Session::locateViews(const LocateViewsOptions &options, ViewState &viewState)
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->locateViews(options, m_viewConfigurationType, viewState);
}

AttachActionSetsResult Session::attachActionSets(const AttachActionSetsOptions &options)
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->attachActionSets(options);
}

InteractionProfileState Session::getInteractionProfile(const GetInterationProfileOptions &options) const
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->getInteractionProfile(options);
}

SyncActionsResult Session::syncActions(const SyncActionsOptions &options)
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->syncActions(options);
}

GetActionStateResult Session::getBooleanState(const GetActionStateOptions &options, ActionStateBoolean &state) const
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->getBooleanState(options, state);
}

GetActionStateResult Session::getFloatState(const GetActionStateOptions &options, ActionStateFloat &state) const
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->getFloatState(options, state);
}

VibrateOutputResult Session::vibrateOutput(const VibrationOutputOptions &options)
{
    auto apiSession = m_api->resourceManager()->getSession(m_session);
    return apiSession->vibrateOutput(options);
}

} // namespace KDXr
