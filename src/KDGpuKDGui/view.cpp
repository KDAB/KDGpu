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

#if defined(KDGUI_PLATFORM_WIN32)
#include <KDGui/platform/win32/win32_platform_window.h>
#endif
#if defined(KDGUI_PLATFORM_XCB)
#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif
#if defined(KDGUI_PLATFORM_WAYLAND)
#include <KDGui/platform/linux/wayland/linux_wayland_platform_window.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_integration.h>
#endif
#if defined(KDGUI_PLATFORM_COCOA)
#include <KDGui/platform/cocoa/cocoa_platform_window.h>
extern CAMetalLayer *createMetalLayer(KDGui::Window *window);
#endif
#if defined(KDGUI_PLATFORM_ANDROID)
#include <KDGui/platform/android/android_platform_window.h>
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
                          })
            .release();
}

View::~View()
{
}

KDGpu::SurfaceOptions View::surfaceOptions(KDGui::Window *w)
{
    KDGui::AbstractPlatformWindow *platformWindow = w->platformWindow();
    if (platformWindow == nullptr)
        return {};

    switch (platformWindow->type()) {
    case KDGui::AbstractPlatformWindow::Type::Win32: {
#if defined(KDGUI_PLATFORM_WIN32)
        auto win32Window = static_cast<KDGui::Win32PlatformWindow *>(w->platformWindow());
        return KDGpu::SurfaceOptions{
            .hWnd = win32Window->handle()
        };
#else
        break;
#endif
    }

    case KDGui::AbstractPlatformWindow::Type::XCB: {
#if defined(KDGUI_PLATFORM_XCB)
        auto xcbWindow = static_cast<KDGui::LinuxXcbPlatformWindow *>(w->platformWindow());
        return KDGpu::SurfaceOptions{
            .connection = xcbWindow->connection(),
            .window = xcbWindow->handle()
        };
#else
        break;
#endif
    }

    case KDGui::AbstractPlatformWindow::Type::Wayland: {
#if defined(KDGUI_PLATFORM_WAYLAND)
        auto waylandWindow = static_cast<KDGui::LinuxWaylandPlatformWindow *>(w->platformWindow());
        return KDGpu::SurfaceOptions{
            .display = waylandWindow->display(),
            .surface = waylandWindow->surface()
        };
#else
        break;
#endif
    }

    case KDGui::AbstractPlatformWindow::Type::Cocoa: {
#if defined(KDGUI_PLATFORM_COCOA)
        return KDGpu::SurfaceOptions{
            .layer = createMetalLayer(w)
        };
#else
        break;
#endif
    }
    case KDGui::AbstractPlatformWindow::Type::Android: {
#if defined(KDGUI_PLATFORM_ANDROID)
        auto androidWindow = static_cast<KDGui::AndroidPlatformWindow *>(w->platformWindow());
        return KDGpu::SurfaceOptions{
            .window = androidWindow->nativeWindow()
        };
#else
        break;
#endif
    }
    }

    return {};
}

KDGpu::Surface View::createSurface(KDGpu::Instance &instance)
{
    KDGpu::Surface surface = instance.createSurface(View::surfaceOptions(this));
    return surface;
}

} // namespace KDGpuKDGui
