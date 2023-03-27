#pragma once

#include <kdgpu_kdgui/example_engine_layer.h>
#include <kdgpu_kdgui/kdgpu_kdgui_export.h>

#include <array>

using namespace KDGpu;

namespace KDGpuKDGui {

class KDGPU_KDGUI_EXPORT AdvancedExampleEngineLayer : public ExampleEngineLayer
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

} // namespace KDGpuKDGui
