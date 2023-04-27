#include "view.h"

#include <KDGpu/instance.h>

#include <KDFoundation/config.h> // for KD_PLATFORM
#include <KDFoundation/core_application.h>

#if defined(KD_PLATFORM_WIN32)
#include <KDGui/platform/win32/win32_platform_window.h>
#endif
#if defined(KD_PLATFORM_LINUX)
#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif
#if defined(KD_PLATFORM_MACOS)
extern CAMetalLayer *createMetalLayer(KDGui::Window *window);
#endif

namespace KDGpuExample {

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
    auto win32Window = dynamic_cast<KDGui::Win32PlatformWindow *>(w->platformWindow());
    return KDGpu::SurfaceOptions{
        .hWnd = win32Window->handle()
    };
#endif

#if defined(KD_PLATFORM_LINUX)
    auto xcbWindow = dynamic_cast<KDGui::LinuxXcbPlatformWindow *>(w->platformWindow());
    return KDGpu::SurfaceOptions{
        .connection = xcbWindow->connection(),
        .window = xcbWindow->handle()
    };
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

} // namespace KDGpuExample
