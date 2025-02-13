/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "system.h"

#include <KDXr/xr_api.h>
#include <KDXr/api/resource_manager_impl.h>

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

    @return KDGpu::Handle<System_t>
    @sa ResourceManager
 */

/**
    @fn System::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

System::System(XrApi *api, const KDGpu::Handle<System_t> &system)
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

ViewConfigurationType System::selectViewConfiguration(std::span<const ViewConfigurationType> preferredViewConfigurations) const
{
    auto availableViewConfigurations = viewConfigurations();
    for (auto preferredViewConfiguration : preferredViewConfigurations) {
        if (std::find(availableViewConfigurations.begin(), availableViewConfigurations.end(), preferredViewConfiguration) != availableViewConfigurations.end()) {
            return preferredViewConfiguration;
        }
    }
    SPDLOG_LOGGER_ERROR(Logger::logger(), "System::selectedViewConfiguration: No preferred view configuration found.");
    return ViewConfigurationType::MaxEnum;
}

std::vector<EnvironmentBlendMode> System::environmentBlendModes(ViewConfigurationType viewConfiguration) const
{
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->queryEnvironmentBlendModes(viewConfiguration);
}

std::vector<ViewConfigurationView> System::views(ViewConfigurationType viewConfiguration) const
{
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->queryViews(viewConfiguration);
}

GraphicsRequirements System::graphicsRequirements() const
{
    if (!m_graphicsApi) {
        throw std::runtime_error("System::requiredGraphicsInstanceExtensions: No graphics API set. Please call setGraphicsApi() first.");
    }
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->queryGraphicsRequirements(m_graphicsApi);
}

std::vector<std::string> System::requiredGraphicsInstanceExtensions() const
{
    if (!m_graphicsApi) {
        throw std::runtime_error("System::requiredGraphicsInstanceExtensions: No graphics API set. Please call setGraphicsApi() first.");
    }
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->requiredGraphicsInstanceExtensions(m_graphicsApi);
}

KDGpu::Adapter *System::requiredGraphicsAdapter(const KDGpu::Instance &graphicsInstance) const
{
    if (!m_graphicsApi) {
        throw std::runtime_error("System::requiredGraphicsInstanceExtensions: No graphics API set. Please call setGraphicsApi() first.");
    }
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->requiredGraphicsAdapter(m_graphicsApi, graphicsInstance);
}

std::vector<std::string> System::requiredGraphicsDeviceExtensions() const
{
    if (!m_graphicsApi) {
        throw std::runtime_error("System::requiredGraphicsInstanceExtensions: No graphics API set. Please call setGraphicsApi() first.");
    }
    auto apiSystem = m_api->resourceManager()->getSystem(m_system);
    return apiSystem->requiredGraphicsDeviceExtensions(m_graphicsApi);
}

Session System::createSession(const SessionOptions &options)
{
    return Session(m_system, m_api, options);
}

} // namespace KDXr
