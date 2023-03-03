#pragma once

#include <toy_renderer_kdgui/simple_example_engine_layer.h>

#include <toy_renderer/bind_group.h>
#include <toy_renderer/buffer.h>
#include <toy_renderer/graphics_pipeline.h>
#include <toy_renderer/render_pass_command_recorder_options.h>

#include <glm/glm.hpp>

using namespace ToyRendererKDGui;

class ComputeParticles : public SimpleExampleEngineLayer
{
public:
protected:
    void initializeScene() override;
    void cleanupScene() override;
    void updateScene() override;
    void render() override;

    void renderSingleCommandBuffer();
    void renderMultipleCommandBuffers();

private:
    Buffer m_particleDataBuffer;
    Buffer m_triangleVertexBuffer;
    ComputePipeline m_computePipeline;
    GraphicsPipeline m_graphicsPipeline;
    PipelineLayout m_graphicsPipelineLayout;
    PipelineLayout m_computePipelineLayout;
    RenderPassCommandRecorderOptions m_opaquePassOptions;
    CommandBuffer m_graphicsCommands;
    CommandBuffer m_computeCommands;
    CommandBuffer m_graphicsAndComputeCommands;

    BindGroup m_particleBindGroup;
    GpuSemaphore m_computeSemaphoreComplete;
};
