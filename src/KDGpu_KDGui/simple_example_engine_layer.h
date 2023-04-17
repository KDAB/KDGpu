#pragma once

#include <KDGpu_KDGui/example_engine_layer.h>
#include <KDGpu_KDGui/kdgpu_kdgui_export.h>

using namespace KDGpu;

namespace KDGpuKDGui {

class KDGPU_KDGUI_EXPORT SimpleExampleEngineLayer : public ExampleEngineLayer
{
public:
    SimpleExampleEngineLayer();
    explicit SimpleExampleEngineLayer(const SampleCountFlagBits samples);
    ~SimpleExampleEngineLayer() override;

protected:
    void update() override;
};

} // namespace KDGpuKDGui
