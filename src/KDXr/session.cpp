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

ReferenceSpace Session::createReferenceSpace(const ReferenceSpaceOptions &options)
{
    return ReferenceSpace(m_session, m_api, options);
}

Session::Session(System *system, XrApi *api, const SessionOptions &options)
    : m_api(api)
    , m_system(system)
{
    // Create an Session using the underlying API
    m_session = m_api->resourceManager()->createSession(options);
}

Session::~Session()
{
    if (isValid())
        m_api->resourceManager()->deleteSession(handle());
}

Session::Session(Session &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_system = std::exchange(other.m_system, nullptr);
    m_session = std::exchange(other.m_session, {});
}

Session &Session::operator=(Session &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteSession(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_system = std::exchange(other.m_system, nullptr);
        m_session = std::exchange(other.m_session, {});
    }
    return *this;
}

} // namespace KDXr
