/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>
#include <KDGpu/ycbcr_conversion_options.h>
#include <KDGpu/graphics_api.h>

namespace KDGpu {

struct YCbCrConversion_t;
struct Device_t;

class KDGPU_EXPORT YCbCrConversion
{
public:
    YCbCrConversion();
    ~YCbCrConversion();

    YCbCrConversion(YCbCrConversion &&) noexcept;
    YCbCrConversion &operator=(YCbCrConversion &&) noexcept;

    YCbCrConversion(const YCbCrConversion &) = delete;
    YCbCrConversion &operator=(const YCbCrConversion &) = delete;

    const Handle<YCbCrConversion_t> &handle() const noexcept { return m_conversion; }
    bool isValid() const noexcept { return m_conversion.isValid(); }

    operator Handle<YCbCrConversion_t>() const noexcept { return m_conversion; }

private:
    YCbCrConversion(GraphicsApi *api, const Handle<Device_t> &device, const YCbCrConversionOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Device_t> m_device;
    Handle<YCbCrConversion_t> m_conversion;

    friend class Device;
    friend KDGPU_EXPORT bool operator==(const YCbCrConversion &, const YCbCrConversion &);
};

KDGPU_EXPORT bool operator==(const YCbCrConversion &a, const YCbCrConversion &b);
KDGPU_EXPORT bool operator!=(const YCbCrConversion &a, const YCbCrConversion &b);

} // namespace KDGpu
