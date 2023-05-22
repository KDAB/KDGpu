/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/kdgpu_export.h>

#include <KDGpu/utils/logging.h>

template<>
struct KDGPU_EXPORT fmt::formatter<KDGpu::AdapterDeviceType> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(KDGpu::AdapterDeviceType const &deviceType, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(), KDGpu::adapterDeviceTypeToString(deviceType));
    }
};
