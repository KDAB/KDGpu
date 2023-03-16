#pragma once

#include <toy_renderer_kdgui/simple_example_engine_layer.h>

#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

using namespace ToyRendererKDGui;

class DynamicUBOTriangles : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;
    void resize() override;

private:
    Buffer m_buffer;
    Buffer m_indexBuffer;
    GraphicsPipeline m_pipeline;
    PipelineLayout m_pipelineLayout;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_commandBuffer;
    Buffer m_transformDynamicUBOBuffer;
    BindGroup m_transformBindGroup;

    size_t m_dynamicUBOByteStride{ 0 };
};
