/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "action.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_action.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class Action
    @brief Action is used to initialize the XR API.
    @ingroup public
    @headerfile action.h <KDXr/action.h>

    @sa ActionOptions
 */

/**
    @fn Action::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific Action

    @return KDGpu::Handle<Action_t>
    @sa ResourceManager
 */

/**
    @fn Action::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

Action::Action()
{
}

Action::Action(XrApi *api, const KDGpu::Handle<ActionSet_t> &actionSetHandle, const ActionOptions &options)
    : m_api(api)
    , m_actionSetHandle(actionSetHandle)
    , m_action(m_api->resourceManager()->createAction(m_actionSetHandle, options))
{
}

Action::~Action()
{
    if (isValid())
        m_api->resourceManager()->deleteAction(handle());
}

Action::Action(Action &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_actionSetHandle = std::exchange(other.m_actionSetHandle, {});
    m_action = std::exchange(other.m_action, {});
}

Action &Action::operator=(Action &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteAction(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_actionSetHandle = std::exchange(other.m_actionSetHandle, {});
        m_action = std::exchange(other.m_action, {});
    }
    return *this;
}

} // namespace KDXr
