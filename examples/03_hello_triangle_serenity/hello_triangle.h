#pragma once

#include "example_engine_layer.h"

#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

class HelloTriangle : public ExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;

private:
    Buffer m_buffer;
    GraphicsPipeline m_pipeline;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
};
