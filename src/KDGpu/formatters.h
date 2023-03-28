#pragma once

#include <KDGpu/gpu_core.h>
#include <KDGpu/kdgpu_export.h>

#include <spdlog/spdlog.h>

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
