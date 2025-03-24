/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 202 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ycbcr_conversion.h"

#include <KDGpu/api/graphics_api_impl.h>

namespace KDGpu {

YCbCrConversion::YCbCrConversion() = default;

YCbCrConversion::~YCbCrConversion()
{
    if (isValid())
        m_api->resourceManager()->deleteYCbCrConversion(handle());
};

YCbCrConversion::YCbCrConversion(GraphicsApi *api, const Handle<Device_t> &device, const YCbCrConversionOptions &options)
    : m_api(api)
    , m_device(device)
    , m_conversion(m_api->resourceManager()->createYCbCrConversion(m_device, options))
{
}

YCbCrConversion::YCbCrConversion(YCbCrConversion &&other) noexcept
{
    m_api = std::exchange(other.m_api, nullptr);
    m_device = std::exchange(other.m_device, {});
    m_conversion = std::exchange(other.m_conversion, {});
}

YCbCrConversion &YCbCrConversion::operator=(YCbCrConversion &&other) noexcept
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteYCbCrConversion(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_device = std::exchange(other.m_device, {});
        m_conversion = std::exchange(other.m_conversion, {});
    }
    return *this;
}

bool operator==(const YCbCrConversion &a, const YCbCrConversion &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_conversion == b.m_conversion;
}

bool operator!=(const YCbCrConversion &a, const YCbCrConversion &b)
{
    return !(a == b);
}

} // namespace KDGpu
