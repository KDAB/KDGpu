/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "system.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_system.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class System
    @brief System is used to initialize the XR API.
    @ingroup public
    @headerfile system.h <KDXr/system.h>

    @sa SystemOptions
 */

/**
    @fn System::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific System

    @return Handle<System_t>
    @sa ResourceManager
 */

/**
    @fn System::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

System::System(XrApi *api, const Handle<System_t> &system)
    : m_api(api)
    , m_system(system)
{
    // TODO: Query system properties
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    m_properties = apiSystem->queryProperties();
    m_viewConfigurations = apiSystem->queryViewConfigurations();
}

System::~System()
{
    if (isValid())
        m_api->resourceManager()->removeSystem(m_system);
}

System::System(System &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_system = std::exchange(other.m_system, {});
    m_properties = std::exchange(other.m_properties, {});
    m_viewConfigurations = std::exchange(other.m_viewConfigurations, {});
}

System &System::operator=(System &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->removeSystem(m_system);

        m_api = std::exchange(other.m_api, nullptr);
        m_system = std::exchange(other.m_system, {});
        m_properties = std::exchange(other.m_properties, {});
        m_viewConfigurations = std::exchange(other.m_viewConfigurations, {});
    }
    return *this;
}

SystemProperties System::properties() const
{
    return m_properties;
}

std::span<const ViewConfigurationType> System::viewConfigurations() const
{
    return m_viewConfigurations;
}

std::vector<EnvironmentBlendMode> System::environmentBlendModes(ViewConfigurationType viewConfiguration) const
{
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->queryEnvironmentBlendModes(viewConfiguration);
}

} // namespace KDXr
