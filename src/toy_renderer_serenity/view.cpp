#include "view.h"

#include <toy_renderer/instance.h>
#include <toy_renderer/surface_options.h>

#include <Serenity/core/core_application.h>

#if defined(TOY_RENDERER_PLATFORM_WIN32)
#include <Serenity/gui/platform/win32/win32_platform_window.h>
#endif
#if defined(TOY_RENDERER_PLATFORM_LINUX)
#include <Serenity/gui/platform/linux/xcb/linux_xcb_platform_window.h>
#endif

namespace ToyRendererSerenity {

View::View()
    : Serenity::Window()
{
    width = 1920;
    height = 1080;
    visible = true;

    auto app = Serenity::CoreApplication::instance();
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
#if defined(TOY_RENDERER_PLATFORM_WIN32)
    auto win32Window = dynamic_cast<Serenity::Win32PlatformWindow *>(platformWindow());
    ToyRenderer::SurfaceOptions surfaceOptions = {
        .hWnd = win32Window->handle()
    };
#endif

#if defined(TOY_RENDERER_PLATFORM_LINUX)
    auto xcbWindow = dynamic_cast<Serenity::LinuxXcbPlatformWindow *>(platformWindow());
    ToyRenderer::SurfaceOptions surfaceOptions = {
        .connection = xcbWindow->connection(),
        .window = xcbWindow->handle()
    };
#endif
    ToyRenderer::Surface surface = instance.createSurface(surfaceOptions);
    return surface;
}

} // namespace ToyRendererSerenity
