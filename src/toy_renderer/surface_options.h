#pragma once

#if defined(TOY_RENDERER_PLATFORM_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

#if defined(TOY_RENDERER_PLATFORM_LINUX)
#endif

#if defined(TOY_RENDERER_PLATFORM_MACOS)
#endif

namespace ToyRenderer {

struct SurfaceOptions {
#if defined(TOY_RENDERER_PLATFORM_WIN32)
    HWND hWnd;
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)
    // TODO: Add Wayland support
    xcb_connection_t *connection;
    xcb_window_t window;
#endif
#if defined(TOY_RENDERER_PLATFORM_MACOS)
    CAMetalLayer *layer;
#endif
};

} // namespace ToyRenderer
