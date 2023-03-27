#pragma once

#include <kdgpu_kdgui/example_engine_layer.h>
#include <kdgpu_kdgui/kdgpu_kdgui_export.h>

using namespace KDGpu;

namespace KDGpuKDGui {

class KDGPU_KDGUI_EXPORT SimpleExampleEngineLayer : public ExampleEngineLayer
{
public:
    SimpleExampleEngineLayer();
    explicit SimpleExampleEngineLayer(const SampleCountFlagBits samples);
    ~SimpleExampleEngineLayer() override;

protected:
    void onAttached() override;
    void update() override;
};

} // namespace KDGpuKDGui
