#pragma once

#include <Serenity/gui/window.h>

#include <toy_renderer/surface.h>

#include <toy_renderer_serenity/toy_renderer_serenity_export.h>

namespace ToyRenderer {
class Instance;
}

namespace ToyRendererSerenity {

class TOY_RENDERER_SERENITY_EXPORT View : public Serenity::Window
{
public:
    View();
    ~View();

    ToyRenderer::Surface createSurface(ToyRenderer::Instance &instance);
};

} // namespace ToyRendererSerenity
