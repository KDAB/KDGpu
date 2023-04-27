#pragma once

#include <KDGpuExample/example_engine_layer.h>
#include <KDGpuExample/kdgpuexample_export.h>

using namespace KDGpu;

namespace KDGpuExample {

class KDGPUEXAMPLE_EXPORT SimpleExampleEngineLayer : public ExampleEngineLayer
{
public:
    SimpleExampleEngineLayer();
    explicit SimpleExampleEngineLayer(const SampleCountFlagBits samples);
    ~SimpleExampleEngineLayer() override;

protected:
    void update() override;
};

} // namespace KDGpuExample
