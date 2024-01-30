/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "action_set.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_action_set.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class ActionSet
    @brief ActionSet is used to initialize the XR API.
    @ingroup public
    @headerfile action_set.h <KDXr/action_set.h>

    @sa ActionSetOptions
 */

/**
    @fn ActionSet::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific ActionSet

    @return Handle<ActionSet_t>
    @sa ResourceManager
 */

/**
    @fn ActionSet::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

ActionSet::ActionSet()
{
}

ActionSet::ActionSet(XrApi *api, const Handle<Instance_t> &instanceHandle, const ActionSetOptions &options)
    : m_api(api)
    , m_instanceHandle(instanceHandle)
    , m_actionSet(m_api->resourceManager()->createActionSet(m_instanceHandle, options))
{
}

ActionSet::~ActionSet()
{
    if (isValid())
        m_api->resourceManager()->deleteActionSet(handle());
}

ActionSet::ActionSet(ActionSet &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_instanceHandle = std::exchange(other.m_instanceHandle, {});
    m_actionSet = std::exchange(other.m_actionSet, {});
}

ActionSet &ActionSet::operator=(ActionSet &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteActionSet(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_instanceHandle = std::exchange(other.m_instanceHandle, {});
        m_actionSet = std::exchange(other.m_actionSet, {});
    }
    return *this;
}

} // namespace KDXr
