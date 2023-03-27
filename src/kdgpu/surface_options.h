#pragma once

#if defined(KDGPU_PLATFORM_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

#if defined(KDGPU_PLATFORM_LINUX)
#include <xcb/xproto.h>
#endif

#if defined(KDGPU_PLATFORM_MACOS)
#ifdef __OBJC__
@class CAMetalLayer;
#else
typedef void CAMetalLayer;
#endif
#endif

namespace KDGpu {

struct SurfaceOptions {
#if defined(KDGPU_PLATFORM_WIN32)
    HWND hWnd;
#endif
#if defined(KDGPU_PLATFORM_LINUX)
    // TODO: Add Wayland support
    xcb_connection_t *connection;
    xcb_window_t window;
#endif
#if defined(KDGPU_PLATFORM_MACOS)
    CAMetalLayer *layer;
#endif
};

} // namespace KDGpu
