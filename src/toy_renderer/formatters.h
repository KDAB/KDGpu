#pragma once

#include <toy_renderer/gpu_core.h>
#include <toy_renderer/toy_renderer_export.h>

#include <spdlog/spdlog.h>

template<>
struct TOY_RENDERER_EXPORT fmt::formatter<ToyRenderer::AdapterDeviceType> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(ToyRenderer::AdapterDeviceType const &deviceType, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(), ToyRenderer::adapterDeviceTypeToString(deviceType));
    }
};
