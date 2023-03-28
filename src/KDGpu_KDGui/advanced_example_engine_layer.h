#pragma once

#include <KDGpu_KDGui/example_engine_layer.h>
#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

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
