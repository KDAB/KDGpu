#pragma once

#include <toy_renderer_kdgui/example_engine_layer.h>
#include <toy_renderer_kdgui/toy_renderer_kdgui_export.h>

#include <array>

using namespace ToyRenderer;

namespace ToyRendererKDGui {

class TOY_RENDERER_KDGUI_EXPORT AdvancedExampleEngineLayer : public ExampleEngineLayer
{
public:
    AdvancedExampleEngineLayer();
    explicit AdvancedExampleEngineLayer(const SampleCountFlagBits samples);
    ~AdvancedExampleEngineLayer() override;

protected:
    void onAttached() override;
    void onDetached() override;
    void update() override;

    bool m_waitForPresentation{ true };
    std::array<Fence, MAX_FRAMES_IN_FLIGHT> m_frameFences;
};

} // namespace ToyRendererKDGui
