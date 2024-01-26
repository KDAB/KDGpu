/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/config.h>

#if defined(KDGPU_PLATFORM_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(KDGPU_PLATFORM_LINUX)
#include <xcb/xproto.h>

struct wl_display;
struct wl_surface;
#endif

#if defined(KDGPU_PLATFORM_APPLE)
#ifdef __OBJC__
@class CAMetalLayer;
#else
typedef void CAMetalLayer;
#endif
#endif

#if defined(KDGPU_PLATFORM_ANDROID)
#include <android/native_window.h>
#endif

namespace KDGpu {

struct SurfaceOptions {
#if defined(KDGPU_PLATFORM_WIN32)
    HWND hWnd;
#endif
#if defined(KDGPU_PLATFORM_LINUX)
    xcb_connection_t *connection;
    xcb_window_t window;

    wl_display *display = nullptr;
    wl_surface *surface = nullptr;
#endif
#if defined(KDGPU_PLATFORM_APPLE)
    CAMetalLayer *layer;
#endif
#if defined(KDGPU_PLATFORM_ANDROID)
    ANativeWindow *window = nullptr;
#endif
};

} // namespace KDGpu
