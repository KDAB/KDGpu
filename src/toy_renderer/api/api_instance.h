#pragma once

#include <toy_renderer/handle.h>

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

#if defined(TOY_RENDERER_PLATFORM_WIN32)
    virtual Handle<Surface_t> createSurface(HWND hWnd) = 0;
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)
    virtual Handle<Surface_t> createSurface(xcb_connection_t *connection, xcb_window_t window) = 0;
#endif
#if defined(TOY_RENDERER_PLATFORM_MACOS)
    virtual Handle<Surface_t> createSurface(CAMetalLayer *layer) = 0;
#endif
#if defined(TOY_RENDERER_PLATFORM_SERENITY)
    virtual Handle<Surface_t> createSurface(Serenity::Window *window) = 0;
#endif
};

} // namespace ToyRenderer
