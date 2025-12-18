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

#include <string>
#include <vector>

namespace KDGpu {

class Surface;
struct Instance_t;

/**
    @brief Holds option fields used for Instance creation
    @ingroup public
    @headerfile instance.h <KDGpu/instance.h>
 */
struct InstanceOptions {
    std::string applicationName{ "KDGpu Application" };
    uint32_t applicationVersion{ KDGPU_MAKE_API_VERSION(0, 1, 0, 0) };
    // Highest version the application is expected to use
    uint32_t apiVersion{ KDGPU_MAKE_API_VERSION(0, 1, 2, 0) };
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

struct AdapterAndDevice {
    Adapter *adapter;
    Device device;
};

class KDGPU_EXPORT Instance
{
public:
    Instance();
    ~Instance();

    Instance(Instance &&) noexcept;
    Instance &operator=(Instance &&) noexcept;

    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;

    [[nodiscard]] Handle<Instance_t> handle() const noexcept { return m_instance; }
    [[nodiscard]] bool isValid() const { return m_instance.isValid(); }

    operator Handle<Instance_t>() const noexcept { return m_instance; }

    [[nodiscard]] std::vector<Extension> extensions() const;

    [[nodiscard]] AdapterAndDevice createDefaultDevice(const Surface &surface,
                                                       AdapterDeviceType deviceType = AdapterDeviceType::Default) const;

    [[nodiscard]] std::vector<Adapter *> adapters() const;
    [[nodiscard]] const std::vector<AdapterGroup> &adapterGroups() const;

    [[nodiscard]] Adapter *selectAdapter(AdapterDeviceType deviceType) const;

    [[nodiscard]] Surface createSurface(const SurfaceOptions &options);

private:
    Instance(GraphicsApi *api, const InstanceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Instance_t> m_instance;
    mutable std::vector<Adapter> m_adapters;
    mutable std::vector<AdapterGroup> m_adapterGroups;

    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
