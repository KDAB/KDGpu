/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "view.h"

#include <KDGpu/instance.h>

#include <KDFoundation/config.h> // for KD_PLATFORM
#include <KDFoundation/core_application.h>
#include <KDGui/config.h>

#if defined(KD_PLATFORM_WIN32)
#include <KDGui/platform/win32/win32_platform_window.h>
#endif
#if defined(KD_PLATFORM_LINUX)
#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif
#if defined(KDGUI_PLATFORM_WAYLAND)
#include <KDGui/platform/linux/wayland/linux_wayland_platform_window.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_integration.h>
#endif
#if defined(KD_PLATFORM_MACOS)
extern CAMetalLayer *createMetalLayer(KDGui::Window *window);
#endif

namespace KDGpuKDGui {

View::View()
    : KDGui::Window()
{
    width = 1920;
    height = 1080;
    visible = true;

    visible.valueChanged().connect([](const bool &visible) {
        if (visible == false) {
            auto app = KDFoundation::CoreApplication::instance();
            app->quit();
        }
    });
}

View::~View()
{
}

KDGpu::SurfaceOptions View::surfaceOptions(KDGui::Window *w)
{
#if defined(KD_PLATFORM_WIN32)
    auto win32Window = static_cast<KDGui::Win32PlatformWindow *>(w->platformWindow());
    return KDGpu::SurfaceOptions{
        .hWnd = win32Window->handle()
    };
#endif

#if defined(KD_PLATFORM_LINUX)
    auto xcbWindow = static_cast<KDGui::LinuxXcbPlatformWindow *>(w->platformWindow());
    if (xcbWindow != nullptr) {
        return KDGpu::SurfaceOptions{
            .connection = xcbWindow->connection(),
            .window = xcbWindow->handle()
        };
    }
#endif

#if defined(KDGUI_PLATFORM_WAYLAND)
    auto waylandWindow = static_cast<KDGui::LinuxWaylandPlatformWindow *>(w->platformWindow());
    if (waylandWindow != nullptr) {
        return KDGpu::SurfaceOptions{
            .display = waylandWindow->display(),
            .surface = waylandWindow->surface()
        };
    }
#endif

#if defined(KD_PLATFORM_MACOS)
    return KDGpu::SurfaceOptions{
        .layer = createMetalLayer(w)
    };
#endif

    return {};
}

KDGpu::Surface View::createSurface(KDGpu::Instance &instance)
{
    KDGpu::Surface surface = instance.createSurface(View::surfaceOptions(this));
    return surface;
}

} // namespace KDGpuKDGui
