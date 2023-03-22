#pragma once

#include <toy_renderer_kdgui/example_engine_layer.h>
#include <toy_renderer_kdgui/toy_renderer_kdgui_export.h>

using namespace ToyRenderer;

namespace ToyRendererKDGui {

class TOY_RENDERER_KDGUI_EXPORT SimpleExampleEngineLayer : public ExampleEngineLayer
{
public:
    SimpleExampleEngineLayer();
    ~SimpleExampleEngineLayer() override;

protected:
    void onAttached() override;
    void update() override;
};

} // namespace ToyRendererKDGui
