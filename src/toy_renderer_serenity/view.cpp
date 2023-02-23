#include "view.h"

#include <toy_renderer/instance.h>
#include <toy_renderer/surface_options.h>

#include <KDFoundation/config.h> // for KD_PLATFORM
#include <KDFoundation/core_application.h>

#if defined(KD_PLATFORM_WIN32)
#include <KDGui/platform/win32/win32_platform_window.h>
#endif
#if defined(KD_PLATFORM_LINUX)
#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif

namespace ToyRendererSerenity {

View::View()
    : KDGui::Window()
{
    width = 1920;
    height = 1080;
    visible = true;

    auto app = KDFoundation::CoreApplication::instance();
    visible.valueChanged().connect([&app](const bool &visible) {
        if (visible == false)
            app->quit();
    });
}

View::~View()
{
}

ToyRenderer::Surface View::createSurface(ToyRenderer::Instance &instance)
{
#if defined(KD_PLATFORM_WIN32)
    auto win32Window = dynamic_cast<KDGui::Win32PlatformWindow *>(platformWindow());
    ToyRenderer::SurfaceOptions surfaceOptions = {
        .hWnd = win32Window->handle()
    };
#endif

#if defined(KD_PLATFORM_LINUX)
    auto xcbWindow = dynamic_cast<KDGui::LinuxXcbPlatformWindow *>(platformWindow());
    ToyRenderer::SurfaceOptions surfaceOptions = {
        .connection = xcbWindow->connection(),
        .window = xcbWindow->handle()
    };
#endif
    ToyRenderer::Surface surface = instance.createSurface(surfaceOptions);
    return surface;
}

} // namespace ToyRendererSerenity
