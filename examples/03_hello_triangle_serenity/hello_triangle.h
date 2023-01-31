#pragma once

#include "example_engine_layer.h"

#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>

class HelloTriangle : public ExampleEngineLayer
{
public:
protected:
    void onAttached() override;
    void onDetached() override;

private:
    Buffer m_buffer;
    GraphicsPipeline m_pipeline;
};
