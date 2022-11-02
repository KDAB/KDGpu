#pragma once

#include <toy_renderer/handle.h>
#include <toy_renderer/surface_options.h>

#include <vector>

#if defined(TOY_RENDERER_PLATFORM_WIN32)
#define NOMINMAX
#include <windows.h>
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)

#endif
#if defined(TOY_RENDERER_PLATFORM_MACOS)

#endif
#if defined(TOY_RENDERER_PLATFORM_SERENITY)
#include <Serenity/gui/window.h>
#endif

namespace ToyRenderer {

struct Adapter_t;
struct Surface_t;

struct ApiInstance {
    virtual std::vector<Handle<Adapter_t>> queryAdapters() = 0;

    virtual Handle<Surface_t> createSurface(const SurfaceOptions &options) = 0;
};

} // namespace ToyRenderer
