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
#include <KDXr/session.h>

#include <span>
#include <vector>

namespace KDGpu {
class Adapter;
class Instance;
class GraphicsApi;
} // namespace KDGpu

namespace KDXr {

struct System_t;
struct XrApi;

/**
    @brief Holds option fields used for System creation
    @ingroup public
    @headerfile system.h <KDXr/system.h>
 */
struct SystemOptions {
    FormFactor formFactor{ FormFactor::HeadMountedDisplay };
};

class KDXR_EXPORT System
{
public:
    System() = default;
    ~System();

    System(System &&);
    System &operator=(System &&);

    System(const System &) = delete;
    System &operator=(const System &) = delete;

    Handle<System_t> handle() const noexcept { return m_system; }
    bool isValid() const { return m_system.isValid(); }

    operator Handle<System_t>() const noexcept { return m_system; }

    SystemProperties properties() const;

    std::span<const ViewConfigurationType> viewConfigurations() const;
    ViewConfigurationType selectViewConfiguration(std::span<ViewConfigurationType> preferredViewConfigurations) const;
    std::vector<EnvironmentBlendMode> environmentBlendModes(ViewConfigurationType viewConfiguration) const;
    std::vector<ViewConfigurationView> views(ViewConfigurationType viewConfiguration) const;

    // Is this enough to abstract away the graphics API via KDGpu?
    void setGraphicsApi(KDGpu::GraphicsApi *graphicsApi) { m_graphicsApi = graphicsApi; }
    KDGpu::GraphicsApi *graphicsApi() const { return m_graphicsApi; }

    GraphicsRequirements graphicsRequirements() const;
    std::vector<std::string> requiredGraphicsInstanceExtensions() const;
    KDGpu::Adapter *requiredGraphicsAdapter(const KDGpu::Instance &graphicsInstance) const;
    std::vector<std::string> requiredGraphicsDeviceExtensions() const;

    Session createSession(const SessionOptions &options);

private:
    explicit System(XrApi *api, const Handle<System_t> &system);

    XrApi *m_api{ nullptr };
    Handle<System_t> m_system;
    SystemProperties m_properties;
    std::vector<ViewConfigurationType> m_viewConfigurations;

    KDGpu::GraphicsApi *m_graphicsApi{ nullptr };

    friend class Instance;
};

} // namespace KDXr
