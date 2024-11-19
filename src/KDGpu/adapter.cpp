/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "adapter.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

/**
    @class Adapter
    @brief Adapter is a representation of a physical hardware device
    @ingroup public
    @headerfile adapter.h <KDGpu/adapter.h>

    Adapter instances are provided by the Instance. The Adapter is used to query what the underlying physical hardware supports.
    In turn, a logical Device can be created from the Adapter.

    @code
    using namespace KDGpu;

    Adapter *selectedAdapter = instance.selectAdapter(AdapterDeviceType::Default);
    if (!selectedAdapter)
        return;

    auto queueTypes = selectedAdapter->queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
    const bool supportsPresentation = selectedAdapter->supportsPresentation(surface, 0);

    if (!supportsPresentation || !hasGraphicsAndCompute)
        return;
    ...
    @endcode

    @sa KDGpu::Instance::createDefaultDevice
    @sa KDGpu::Instance::adapters
    @sa KDGpu::Instance::selectAdapter
    @sa KDGpu::Device
 */

/**
    @fn Adapter::handle()
    @brief Returns the handle used to retrieve the underlying API specific Adapter

    @sa ResourceManager
 */

/**
    @fn Adapter::isValid()
    @brief Convenience function to check whether the Adapter is actually referencing a valid API specific resource
 */
Adapter::Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter)
    : m_api(api)
    , m_adapter(adapter)
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    m_features = apiAdapter->queryAdapterFeatures();
    m_properties = apiAdapter->queryAdapterProperties();
    m_queueTypes = apiAdapter->queryQueueTypes();
}

Adapter::~Adapter()
{
    if (isValid())
        m_api->resourceManager()->removeAdapter(m_adapter);
}

Adapter::Adapter(Adapter &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_adapter = std::exchange(other.m_adapter, {});
    m_features = std::exchange(other.m_features, {});
    m_properties = std::exchange(other.m_properties, {});
    m_queueTypes = std::exchange(other.m_queueTypes, {});
}

Adapter &Adapter::operator=(Adapter &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->removeAdapter(m_adapter);

        m_api = std::exchange(other.m_api, nullptr);
        m_adapter = std::exchange(other.m_adapter, {});
        m_features = std::exchange(other.m_features, {});
        m_properties = std::exchange(other.m_properties, {});
        m_queueTypes = std::exchange(other.m_queueTypes, {});
    }
    return *this;
}

std::vector<Extension> Adapter::extensions() const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->extensions();
}

/**
 * @brief Returns the AdapterFeatures supported by the Adapter
 */
const AdapterProperties &Adapter::properties() const noexcept
{
    return m_properties;
}

/**
 * @brief Returns the AdapterFeatures supported by the Adapter
 */
const AdapterFeatures &Adapter::features() const noexcept
{
    return m_features;
}

/**
 * @brief Returns the AdapterQueueType supported by the Adapter
 */
std::span<AdapterQueueType> Adapter::queueTypes() const
{
    return m_queueTypes;
}

/**
 * @brief Returns the AdapterSwapchainProperties supported for Surface @a surface
 */
AdapterSwapchainProperties Adapter::swapchainProperties(const Handle<Surface_t> &surface) const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->querySwapchainProperties(surface);
}

/**
 * @brief Returns whether presentation is supported for @a surface and @a queueTypeIndex
 */
bool Adapter::supportsPresentation(const Handle<Surface_t> &surface, uint32_t queueTypeIndex) const noexcept
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->supportsPresentation(surface, queueTypeIndex);
}

/**
 * @brief Returns the FormatProperties for Format @format supported by the Adapter
 */
FormatProperties Adapter::formatProperties(Format format) const
{
    auto apiAdapter = m_api->resourceManager()->getAdapter(m_adapter);
    return apiAdapter->formatProperties(format);
}

/**
    @brief Create a Device object

    @param options
    @return Device
 */
Device Adapter::createDevice(const DeviceOptions &options)
{
    return Device(this, m_api, options);
}

bool Adapter::supportsBlitting(Format srcFormat, TextureTiling srcTiling,
                               Format dstFormat, TextureTiling dstTiling) const
{
    auto featureFlags = [this](Format f, TextureTiling t) {
        const FormatProperties &props = formatProperties(f);
        if (t == TextureTiling::Linear)
            return props.linearTilingFeatures;
        return props.optimalTilingFeatures;
    };

    const FormatFeatureFlags srcFormatFeatureFlags = featureFlags(srcFormat, srcTiling);
    if (!srcFormatFeatureFlags.testFlag(FormatFeatureFlagBit::BlitSrcBit))
        return false;

    const FormatFeatureFlags dstFormatFeatureFlags = featureFlags(dstFormat, dstTiling);
    return dstFormatFeatureFlags.testFlag(FormatFeatureFlagBit::BlitDstBit);
}

bool Adapter::supportsBlitting(Format format, TextureTiling tiling) const
{
    const FormatProperties &props = formatProperties(format);
    const FormatFeatureFlags &formatFeatureFlags = (tiling == TextureTiling::Linear) ? props.linearTilingFeatures : props.optimalTilingFeatures;

    return bool(formatFeatureFlags & (FormatFeatureFlagBit::BlitSrcBit | FormatFeatureFlagBit::BlitDstBit));
}

} // namespace KDGpu
