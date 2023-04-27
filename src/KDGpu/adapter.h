/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_features.h>
#include <KDGpu/adapter_properties.h>
#include <KDGpu/adapter_queue_type.h>
#include <KDGpu/adapter_swapchain_properties.h>
#include <KDGpu/device.h>
#include <KDGpu/handle.h>

#include <KDGpu/kdgpu_export.h>

#include <stdint.h>
#include <span>
#include <string>
#include <vector>

namespace KDGpu {

class GraphicsApi;

struct Adapter_t;
struct Instance_t;
struct Surface_t;

struct AdapterOptions {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class KDGPU_EXPORT Adapter
{
public:
    Adapter() = default;
    ~Adapter();

    Adapter(Adapter &&);
    Adapter &operator=(Adapter &&);

    Adapter(const Adapter &) = delete;
    Adapter &operator=(const Adapter &) = delete;

    Handle<Adapter_t> handle() const noexcept { return m_adapter; }
    bool isValid() const noexcept { return m_adapter.isValid(); }

    operator Handle<Adapter_t>() const noexcept { return m_adapter; }

    std::vector<Extension> extensions() const;
    const AdapterProperties &properties() const noexcept;
    const AdapterFeatures &features() const noexcept;
    std::span<AdapterQueueType> queueTypes() const;

    AdapterSwapchainProperties swapchainProperties(const Handle<Surface_t> &surface) const;
    bool supportsPresentation(const Handle<Surface_t> &surface, uint32_t queueTypeIndex) const noexcept;

    FormatProperties formatProperties(Format format) const;

    Device createDevice(const DeviceOptions &options = DeviceOptions());

private:
    explicit Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter);

    GraphicsApi *m_api{ nullptr };
    Handle<Adapter_t> m_adapter;

    mutable AdapterProperties m_properties;
    mutable bool m_propertiesQueried{ false };

    mutable AdapterFeatures m_features;
    mutable bool m_featuresQueried{ false };

    mutable std::vector<AdapterQueueType> m_queueTypes;

    friend class Instance;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
