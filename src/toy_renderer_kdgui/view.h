#pragma once

#include <KDGui/window.h>

#include <toy_renderer/surface.h>
#include <toy_renderer/surface_options.h>

#include <toy_renderer_kdgui/toy_renderer_kdgui_export.h>

namespace ToyRenderer {
class Instance;
}

namespace ToyRendererKDGui {

class TOY_RENDERER_KDGUI_EXPORT View : public KDGui::Window
{
public:
    View();
    ~View();

    static ToyRenderer::SurfaceOptions surfaceOptions(KDGui::Window *w);
    ToyRenderer::Surface createSurface(ToyRenderer::Instance &instance);
};

} // namespace ToyRendererKDGui
