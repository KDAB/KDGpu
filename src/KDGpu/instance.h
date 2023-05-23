/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter.h>
#include <KDGpu/device.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/surface.h>
#include <KDGpu/surface_options.h>

#include <KDGpu/kdgpu_export.h>

#include <span>
#include <string>
#include <vector>

namespace KDGpu {

class GraphicsApi;
class Surface;
struct Instance_t;

struct InstanceOptions {
    std::string applicationName{ "Serenity Application" };
    uint32_t applicationVersion{ SERENITY_MAKE_API_VERSION(0, 1, 0, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

struct AdapterAndDevice {
    Adapter *adapter;
    Device device;
};

/**
 * @brief Instance
 * @ingroup public
 */
class KDGPU_EXPORT Instance
{
public:
    Instance();
    ~Instance();

    Instance(Instance &&);
    Instance &operator=(Instance &&);

    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;

    Handle<Instance_t> handle() const noexcept { return m_instance; }
    bool isValid() const { return m_instance.isValid(); }

    operator Handle<Instance_t>() const noexcept { return m_instance; }

    std::vector<Extension> extensions() const;

    AdapterAndDevice createDefaultDevice(const Surface &surface,
                                         AdapterDeviceType deviceType = AdapterDeviceType::Default) const;

    std::vector<Adapter *> adapters() const;
    Adapter *selectAdapter(AdapterDeviceType deviceType) const;

    // TODO: Support Serenity::Window, QWindow etc
    //
    // We could provide a tiny library that links to both Serenity and ToyRenderer
    // and exposed SerenityVulkanGraphicsApi which creates a SerenityInstance class that inherits
    // Instance and which creates a Surface from a Serenity::Window. This approach would keep
    // ToyRenderer separate from Serenity/Qt.
    Surface createSurface(const SurfaceOptions &options);

private:
    Instance(GraphicsApi *api, const InstanceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Instance_t> m_instance;
    mutable std::vector<Adapter> m_adapters;

    friend class GraphicsApi;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
