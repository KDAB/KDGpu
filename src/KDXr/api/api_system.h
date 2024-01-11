/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDXr/kdxr_core.h>
#include <KDXr/handle.h>

#include <vector>

namespace KDGpu {
class Adapter;
class GraphicsApi;
class Instance;
} // namespace KDGpu

namespace KDXr {

struct System_t;

/**
 * @brief ApiSystem
 * \ingroup api
 *
 */
struct ApiSystem {
    virtual SystemProperties queryProperties() const = 0;
    virtual std::vector<ViewConfigurationType> queryViewConfigurations() const = 0;
    virtual std::vector<EnvironmentBlendMode> queryEnvironmentBlendModes(ViewConfigurationType viewConfiguration) const = 0;
    virtual std::vector<ViewConfigurationView> queryViews(ViewConfigurationType viewConfiguration) const = 0;
    virtual GraphicsRequirements queryGraphicsRequirements(KDGpu::GraphicsApi *graphicsApi) const = 0;
    virtual std::vector<std::string> requiredGraphicsInstanceExtensions(KDGpu::GraphicsApi *graphicsApi) const = 0;
    virtual KDGpu::Adapter *requiredGraphicsAdapter(KDGpu::GraphicsApi *graphicsApi, const KDGpu::Instance &graphicsInstance) const = 0;
    virtual std::vector<std::string> requiredGraphicsDeviceExtensions(KDGpu::GraphicsApi *graphicsApi) const = 0;
};

} // namespace KDXr
