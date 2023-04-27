#pragma once

#include <KDGpuExample/example_engine_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

#include <array>

using namespace KDGpu;

namespace KDGpuExample {

class KDGPUEXAMPLE_EXPORT AdvancedExampleEngineLayer : public ExampleEngineLayer
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

} // namespace KDGpuExample
